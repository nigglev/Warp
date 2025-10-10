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
	NextUnitID = 1;
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
}

void AWarpGameState::CreateUnitForNewPlayer(APlayerState* OwnerPS)
{
	const uint32 AssignedID = NextUnitID++;
	UUnitBase* Unit = UUnitBase::CreateUnit(this, AssignedID, FUnitSize::Small(), FUnitRotation::Rot45());
	if (!Unit)
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
				TEXT("Failed to create unit (HasAuthority = %d)"), HasAuthority());
	Unit->SetOwningPlayer(OwnerPS);
	Unit->InitStats(EUnitAffiliation::PlayerControlled, /*Speed*/ 10, /*MaxAP*/ 3);

	StaticCombatMap->PlaceUnitOnMapRand(Unit);
	AddNewUnit(Unit);
}

bool AWarpGameState::TryCreateUnitAtForOwner(APlayerState* OwnerPS, const FIntPoint& CenterGrid, const FUnitRotation& Rotation,
	const FUnitSize& Size, UUnitBase*& OutUnit)
{
	MG_FUNC_LABEL(AWarpGameStateLog);
	OutUnit = nullptr;
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("TryCreateUnitAtForTeam; HasAuthority = %d"),
		HasAuthority());
	
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
	TArray<FIntPoint> Blockers;
	if (!CheckPositionForUnitWithCombatMap(CenterGrid, Rotation, Size, Blockers) || Blockers.Num() > 0)
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("Invalid Tile (HasAuthority = %d)"), HasAuthority());
		return false;
	}
	UUnitBase* NewUnit = UUnitBase::CreateUnit(this, NextUnitID++, Size, Rotation);

	if (!IsValid(NewUnit))
	{
		MG_COND_ERROR(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),
		TEXT("Failed to create unit (HasAuthority = %d)"), HasAuthority());
	}

	if (OwnerPS)
	{
		NewUnit->SetOwningPlayer(OwnerPS);
		NewUnit->InitStats(EUnitAffiliation::PlayerControlled, /*Speed*/ 12, /*MaxAP*/ 3);
	}
	else
	{
		NewUnit->InitStats(EUnitAffiliation::EnemyAI, /*Speed*/ 8, /*MaxAP*/ 3);
	}
	
	FCombatMapTile CenterTile = StaticCombatMap->MakeTile(CenterGrid.X, CenterGrid.Y);
	StaticCombatMap->PlaceUnitAt(NewUnit, CenterTile);

	AddNewUnit(NewUnit);
	if (TurnManager->GetCurrentTurnPhase() == ETurnPhase::InTurn)
		TurnManager->RebuildTurnOrder();

	OutUnit = NewUnit;
	
	return true;
}

void AWarpGameState::AddNewUnit(UUnitBase* InNewUnit)
{
	AddReplicatedSubObject(InNewUnit);
	ActiveUnits.Add(InNewUnit);
	MARK_PROPERTY_DIRTY_FROM_NAME(AWarpGameState, ActiveUnits, this);
	MG_COND_LOG(AWarpGameStateLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("Added Unit %s to Place Units; [%d]; HasAuthority = %d"),
		*InNewUnit->ToString(), ActiveUnits.Num(), HasAuthority());
}

UUnitBase* AWarpGameState::FindUnitByID(const uint32 InID) const
{
	for (UUnitBase* Unit : ActiveUnits)
	{
		if (Unit && Unit->GetUnitID() == InID)
		{
			return Unit;
		}
	}
	return nullptr;
}

bool AWarpGameState::CheckPositionForUnitWithCombatMap(const FIntPoint& InUnitCenter, const FUnitRotation& InUnitRotation, const FUnitSize& InUnitSize, TArray<FIntPoint>& OutBlockers) const
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

