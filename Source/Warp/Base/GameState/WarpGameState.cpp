// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpGameState.h"

#include "EngineUtils.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Base/GameMode/DefaultGameMode.h"
#include "Warp/CombatMap/CombatMap.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/Units/UnitBase.h"
DEFINE_LOG_CATEGORY_STATIC(AWarpGameStateLog, Log, All);

AWarpGameState::AWarpGameState()
{
	bReplicateUsingRegisteredSubObjectList = true;
	StaticCombatMap = CreateDefaultSubobject<UCombatMap>(TEXT("CombatMap"));
	TurnManager = CreateDefaultSubobject<UTurnBasedSystemManager>(TEXT("TurnManager"));
	NextUnitCombatID = 1;
}

void AWarpGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	if (HasAuthority())
	{
		PreLoginInit();
	}
}

void AWarpGameState::PreLoginInit()
{
	MG_COND_ERROR(AWarpGameStateLog, !IsValid(StaticCombatMap) && MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
			TEXT("StaticCombatMap is INVALID %d)"), HasAuthority());
	
	MG_COND_ERROR(AWarpGameStateLog, !IsValid(TurnManager) && MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
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
	DOREPLIFETIME_WITH_PARAMS_FAST(AWarpGameState, bCombatStarted, RepParams);
}

void AWarpGameState::SetCombatStarted(bool bStarted)
{
	if (!HasAuthority())
	{
		return;
	}

	if (bCombatStarted == bStarted)
	{
		return;
	}

	bCombatStarted = bStarted;
	
	MARK_PROPERTY_DIRTY_FROM_NAME(AWarpGameState, bCombatStarted, this);
}

void AWarpGameState::CreateUnitAtRandomPosition(const FUnitDefinition* InUnitDefinition, const EUnitAffiliation InAffiliation)
{
	UUnitBase* Unit = CreateUnit(InUnitDefinition, InAffiliation);
	Unit->UnitRotation.SetRandomRotation();
	StaticCombatMap->PlaceUnitOnMapRand(Unit);
	ProcessNewUnit(Unit);
}

bool AWarpGameState::CreateUnitAt(const FUnitRecord& InUnitRecord, const EUnitAffiliation InAffiliation, const FIntVector2& InGridPosition,
	const FUnitRotation& Rotation, UUnitBase*& OutUnit)
{
	// UUnitBase* Unit = CreateUnit(InUnitRecord, InAffiliation);
	// Unit->UnitRotation = Rotation;
	// TArray<FIntPoint> Blockers;
	//
	// if (!CheckPositionForUnitWithCombatMap(InGridPosition, Unit->UnitRotation, Unit->UnitSize_, Blockers))
	// {
	// 	MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
	// 	TEXT("Invalid Tile (HasAuthority = %d)"), HasAuthority());
	// 	return false;
	// }
	// StaticCombatMap->PlaceUnitAt(Unit, InGridPosition);
	//
	// ProcessNewUnit(Unit);
	// if (TurnManager->GetCurrentTurnPhase() == ETurnPhase::InTurn)
	// 	TurnManager->RebuildTurnOrder();
	//
	// OutUnit = Unit;
	//
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


UUnitBase* AWarpGameState::CreateUnit(const FUnitDefinition* InUnitDefinition, const EUnitAffiliation InAffiliation)
{
	RETURN_ON_FAIL_NULL(AWarpGameStateLog, HasAuthority());
	RETURN_ON_FAIL_NULL(AWarpGameStateLog, IsValid(StaticCombatMap));
	
	const uint32 AssignedCombatID = NextUnitCombatID++;

	UUnitBase* NewUnit = UUnitBase::CreateUnit(this, InUnitDefinition, AssignedCombatID, InAffiliation);

	MG_COND_ERROR(AWarpGameStateLog, !IsValid(NewUnit), TEXT("Failed to create unit (HasAuthority = %d)"), HasAuthority());

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

UUnitBase* AWarpGameState::GetUnitByID(uint32 InUnitID)
{
	if (InUnitID <= 0)
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("Invalid ID; HasAuthority = %d"), HasAuthority());
		return nullptr;
	}
	UUnitBase* const* FoundPtr = ActiveUnits.FindByPredicate(
		[InUnitID](const UUnitBase* Unit)
		{
			return Unit && Unit->GetUnitCombatID() == InUnitID;
		});

	UUnitBase* FoundUnit = FoundPtr ? *FoundPtr : nullptr;
	return FoundUnit;
}

void AWarpGameState::CheckValidState()
{
	if (bClientValidState_)
		return;

	if (!IsClientValidState())
		return;

	bClientValidState_ = true;

	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState), TEXT("Valid State"));
	OnWarpGameStateValid.Broadcast(this);
}

void AWarpGameState::OnRep_SpaceCombatGrid()
{
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("SpaceCombatGrid = %p; GridSize: %u"),	StaticCombatMap, StaticCombatMap == nullptr ? 0 : StaticCombatMap->GetGridSize());
	
	CheckValidState();
}

void AWarpGameState::OnRep_TurnBasedSystemManager()
{
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
	TEXT("OnRep_TurnBasedSystemManager"));

	CheckValidState();
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

void AWarpGameState::OnRep_CombatStarted()
{
	if (bCombatStarted)
	{
		MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("Battle Started"));
		OnCombatStarted.Broadcast();
	}
}

