// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpGameState.h"

#include "EngineUtils.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Base/GameInstanceSubsystem/UnitDataSubsystem.h"
#include "Warp/Base/GameMode/DefaultGameMode.h"
#include "Warp/CombatMap/CombatMap.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/Units/UnitBase.h"
#include "Warp/UnitStaticData/PlayFabUnitTypes.h"
#include "Warp/UnitStaticData/UnitCatalogDTO.h"
DEFINE_LOG_CATEGORY_STATIC(AWarpGameStateLog, Log, All);

AWarpGameState::AWarpGameState()
{
	bReplicateUsingRegisteredSubObjectList = true;
	StaticCombatMap = CreateDefaultSubobject<UCombatMap>(TEXT("CombatMap"));
	TurnManager = CreateDefaultSubobject<UTurnBasedSystemManager>(TEXT("TurnManager"));
	NextUnitCombatID = 1;
}

void AWarpGameState::PreLoginInit()
{
	if (!StaticCombatMap)
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
			TEXT("StaticCombatMap is INVALID %d)"), HasAuthority());
	if (!TurnManager)
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
			TEXT("TurnManager is INVALID %d)"), HasAuthority());

	AddReplicatedSubObject(StaticCombatMap);
	AddReplicatedSubObject(TurnManager);
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("Creating Player Static Combat Map and Turn Manager On Server %s; HasAuthority = %d"),
		*StaticCombatMap->GetName(), HasAuthority());
}

void AWarpGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(AWarpGameState, StaticCombatMap, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AWarpGameState, TurnManager, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AWarpGameState, ActiveUnits, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(AWarpGameState, UnitCatalog, RepParams);
}

void AWarpGameState::SetUnitCatalogFromMap(const TMap<FName, FUnitRecord>& Source)
{
	if (!HasAuthority())
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("No Server (HasAuthority = %d)"), HasAuthority());
		return;
	}
	UnitCatalog.Reset(Source.Num());
	for (const auto& UnitRecord : Source)
	{
		FName UnitName = UnitRecord.Key;
		FUnitRecord Rec = UnitRecord.Value;

		FUnitRecordDTO DTO;
		DTO.UnitName  = UnitName;
		DTO.UnitSize  = Rec.Props.UnitSize;
		DTO.UnitSpeed = Rec.Props.UnitSpeed;
		DTO.UnitMaxAP = Rec.Props.UnitMaxAP;
		DTO.MeshPath  = Rec.Props.MeshPath.ToString();
		DTO.Tags      = Rec.Props.Tags;

		UnitCatalog.Add(MoveTemp(DTO));
	}
	
	MARK_PROPERTY_DIRTY_FROM_NAME(AWarpGameState, UnitCatalog, this);
}

void AWarpGameState::CreateUnitAtRandomPosition(const FUnitRecord& InUnitRecord, const EUnitAffiliation InAffiliation)
{
	UUnitBase* Unit = CreateUnit(InUnitRecord, InAffiliation);
	Unit->UnitRotation.SetRandomRotation();
	StaticCombatMap->PlaceUnitOnMapRand(Unit);
	ProcessNewUnit(Unit);
}

bool AWarpGameState::CreateUnitAt(const FUnitRecord& InUnitRecord, const EUnitAffiliation InAffiliation, const FIntVector2& InGridPosition,
	const FUnitRotation& Rotation, UUnitBase*& OutUnit)
{
	UUnitBase* Unit = CreateUnit(InUnitRecord, InAffiliation);
	Unit->UnitRotation = Rotation;
	TArray<FIntPoint> Blockers;
	
	if (!CheckPositionForUnitWithCombatMap(InGridPosition, Unit->UnitRotation, Unit->UnitSize, Blockers))
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("Invalid Tile (HasAuthority = %d)"), HasAuthority());
		return false;
	}
	StaticCombatMap->PlaceUnitAt(Unit, InGridPosition);

	ProcessNewUnit(Unit);
	if (TurnManager->GetCurrentTurnPhase() == ETurnPhase::InTurn)
		TurnManager->RebuildTurnOrder();

	OutUnit = Unit;
	
	return true;
}

bool AWarpGameState::MoveUnitTo(const uint32 InUnitToMoveID, const FIntVector2& InGridPosition)
{
	if (!HasAuthority())
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("No Server (HasAuthority = %d)"), HasAuthority());
		return false;
	}
	if (!IsValid(StaticCombatMap))
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("No Map (HasAuthority = %d)"), HasAuthority());
		return false;
	}

	if (UUnitBase** Found = Algo::FindBy(ActiveUnits, InUnitToMoveID, &UUnitBase::GetUnitCombatID))
	{
		UUnitBase* Unit = *Found;
		Unit->UnitPosition.SetUnitTilePosition(InGridPosition);
		return true;
	}

	return false;
}


UUnitBase* AWarpGameState::CreateUnit(const FUnitRecord& InUnitRecord, const EUnitAffiliation InAffiliation)
{
	if (!HasAuthority())
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("No Server (HasAuthority = %d)"), HasAuthority());
		return nullptr;
	}
	if (!IsValid(StaticCombatMap))
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("No Map (HasAuthority = %d)"), HasAuthority());
		return nullptr;
	}
		
	
	const UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	RETURN_ON_FAIL_NULL(AWarpGameStateLog, Sys);
	
	const uint32 AssignedCombatID = NextUnitCombatID++;
	const FName UnitTypeName = InUnitRecord.UnitName;
	EUnitSizeCategory UnitSize = InUnitRecord.GetSizeCategory();
	uint32 UnitSpeed = InUnitRecord.Props.UnitSpeed;
	uint32 UnitMaxAP = InUnitRecord.Props.UnitMaxAP;

	UUnitBase* NewUnit = UUnitBase::CreateUnit(this, UnitTypeName, AssignedCombatID, InAffiliation, UnitSize);

	if (!IsValid(NewUnit))
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("Failed to create unit (HasAuthority = %d)"), HasAuthority());
		return nullptr;
	}

	NewUnit->InitStats(UnitSpeed, UnitMaxAP);

	return NewUnit;
}

void AWarpGameState::ProcessNewUnit(UUnitBase* InNewUnit)
{
	AddReplicatedSubObject(InNewUnit);
	ActiveUnits.Add(InNewUnit);
	MARK_PROPERTY_DIRTY_FROM_NAME(AWarpGameState, ActiveUnits, this);
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("Added Unit %s to Place Units; [%d]; HasAuthority = %d"),
		*InNewUnit->ToString(), ActiveUnits.Num(), HasAuthority());
}


UUnitBase* AWarpGameState::FindUnitByCombatID(const uint32 InCombatID) const
{
	for (UUnitBase* Unit : ActiveUnits)
	{
		if (Unit && Unit->GetUnitCombatID() == InCombatID)
		{
			return Unit;
		}
	}
	return nullptr;
}

bool AWarpGameState::CheckPositionForUnitWithCombatMap(const FIntVector2& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const
{
	return StaticCombatMap->CheckPositionForUnit(InUnitCenter, InUnitRotation, InUnitSize, OutBlockers);
}

uint32 AWarpGameState::GetMapGridSize() const
{
	return StaticCombatMap->GetGridSize();
}

uint32 AWarpGameState::GetMapTileSize() const
{
	return StaticCombatMap->GetTileSize();
}

UUnitDataSubsystem* AWarpGameState::GetUnitDataSubsystem(const UObject* WorldContext)
{
	if (!WorldContext) return nullptr;
	if (const UWorld* World = WorldContext->GetWorld())
	{
		if (UGameInstance* GI = World->GetGameInstance())
		{
			return GI->GetSubsystem<UUnitDataSubsystem>();
		}
	}
	return nullptr;
}

void AWarpGameState::OnRep_SpaceCombatGrid()
{
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("SpaceCombatGrid = %p; GridSize: %u"),	StaticCombatMap, StaticCombatMap == nullptr ? 0 : StaticCombatMap->GetGridSize());
}

void AWarpGameState::OnRep_TurnBasedSystemManager()
{
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
	TEXT("OnRep_TurnBasedSystemManager"));
}

void AWarpGameState::OnRep_ActiveUnits()
{
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("Replicated ActiveUnits; Number of units: %d"), ActiveUnits.Num());
	for (int i = 0; i < ActiveUnits.Num(); i++)
	{
		if (ActiveUnits[i] == nullptr)
			continue;
		MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
			TEXT("%s; "), ActiveUnits[i] ? *ActiveUnits[i]->ToString() : TEXT("NULL"));
	}
	OnUnitsReplicated.Broadcast(ActiveUnits);
}

void AWarpGameState::OnRep_UnitCatalog()
{
	UUnitDataSubsystem* Sys = GetUnitDataSubsystem(this);
	RETURN_ON_FAIL(AWarpGameStateLog, Sys);
	Sys->SetUnitCatalog_Client(UnitCatalog);
}

