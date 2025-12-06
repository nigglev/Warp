// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnBasedSystemManager.h"

#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Warp/Base/GameState/WarpGameState.h"

DEFINE_LOG_CATEGORY_STATIC(UTurnBasedSystemManagerLog, Log, All);

void UTurnBasedSystemManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(UTurnBasedSystemManager, TurnOrderUnitIds, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UTurnBasedSystemManager, CurrentTurnUnitId, RepParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UTurnBasedSystemManager, Phase, RepParams);
}

void UTurnBasedSystemManager::StartCombat()
{
	if (!GetGameState() || !GetGameState()->HasAuthority() || bTurnsStarted)
		return;
	BuildTurnOrder();

	if (TurnOrderUnitIds.Num() == 0)
	{
		MG_COND_ERROR(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
		TEXT("StartTurns: no units."));
	}
	
	ActiveUnitIndex = 0;
	CurrentTurnUnitId = TurnOrderUnitIds[ActiveUnitIndex];
	MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, CurrentTurnUnitId, this);
}

void UTurnBasedSystemManager::AdvanceToNextUnit()
{
	if (!GetGameState() || !GetGameState()->HasAuthority() || TurnOrderUnitIds.Num() == 0) return;
	ActiveUnitIndex = (ActiveUnitIndex + 1) % TurnOrderUnitIds.Num();
	CurrentTurnUnitId = TurnOrderUnitIds[ActiveUnitIndex];
	MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, CurrentTurnUnitId, this);
}

bool UTurnBasedSystemManager::IsEnoughActionForMovement(const FIntVector2& InDistanceToTargetPosition)
{
	int32 dx = FMath::Abs(InDistanceToTargetPosition.X);
	int32 dy = FMath::Abs(InDistanceToTargetPosition.Y);
	int32 Steps = FMath::Max(dx, dy);

	bool bCanMove = GetCurrentUnitActionPoints() >= Steps;
	if (!bCanMove)
		MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
			TEXT("Not enough action points to move. Need: %d action points; Currently have: %d action points"), Steps, GetCurrentUnitActionPoints());

	return GetCurrentUnitActionPoints() >= Steps;
}


void UTurnBasedSystemManager::BuildTurnOrder()
{
	if (!GetGameState() || !GetGameState()->HasAuthority())
		return;
	
	SortTurnOrder();
	
	MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("Current turn order on server {%d}: "), GetGameState()->HasAuthority());
	for (int i = 0; i < TurnOrderUnitIds.Num(); i++)
	{
		MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("%d") , TurnOrderUnitIds[i]);
	}

}

void UTurnBasedSystemManager::RebuildTurnOrder()
{
	if (!GetGameState() || !GetGameState()->HasAuthority())
		return;
	
	if (!bTurnsStarted)
	{
		BuildTurnOrder();
		return;
	}

	const uint32 PrevCurrent = CurrentTurnUnitId;

	SortTurnOrder();

	const int32 NewIdx = TurnOrderUnitIds.IndexOfByKey(PrevCurrent);
	if (NewIdx != INDEX_NONE)
	{
		ActiveUnitIndex = NewIdx;
	}
	else
	{
		ActiveUnitIndex = 0;
		CurrentTurnUnitId = TurnOrderUnitIds.Num() ? TurnOrderUnitIds[0] : 0;
		MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, CurrentTurnUnitId, this);
		
	}
	OnTurnOrderUpdated.Broadcast(TurnOrderUnitIds, CurrentTurnUnitId);
}

void UTurnBasedSystemManager::SortTurnOrder()
{
	if (!GetGameState() || !GetGameState()->HasAuthority())
		return;
	
	TArray<UUnitBase*> Copy = GetGameState()->GetActiveUnits();
	Copy.RemoveAll([](const UUnitBase* U){ return U == nullptr; });

	Copy.Sort([&](const UUnitBase& A, const UUnitBase& B)
	{
		return A.GetSpeed() > B.GetSpeed();
	});

	TurnOrderUnitIds.Reset(Copy.Num());
	for (const UUnitBase* U : Copy)
		TurnOrderUnitIds.Add(U->GetUnitCombatID());
	MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, TurnOrderUnitIds, this);
}

AWarpGameState* UTurnBasedSystemManager::GetGameState() const
{
	return GetTypedOuter<AWarpGameState>();
}

int32 UTurnBasedSystemManager::GetCurrentUnitActionPoints()
{
	UUnitBase* U = GetGameState()->GetUnitByID(GetActiveUnitID());
	if (!IsValid(U))
	{
		MG_COND_ERROR(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
		TEXT("Invalid UNIT."));
		return -1;
	}
	return U->GetCurrentAP();
}


uint32 UTurnBasedSystemManager::GetActiveUnitID() const
{
	return CurrentTurnUnitId;
}

uint32 UTurnBasedSystemManager::GetFirstUnitIDInOrder() const
{
	if (TurnOrderUnitIds.Num() == 0)
	{
		MG_COND_ERROR(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
		TEXT("StartTurns: no units."));
	}

	return TurnOrderUnitIds[0];
}


void UTurnBasedSystemManager::OnRep_CurrentTurn()
{
	MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
	TEXT("CurrentTurn %d"),	CurrentTurnUnitId);
	OnActiveUnitChanged.Broadcast(CurrentTurnUnitId);
}

void UTurnBasedSystemManager::OnRep_CurrentPhase()
{
	OnTurnPhaseChanged.Broadcast(Phase);
	MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
	TEXT("CurrentPhase %d"), Phase);
}

void UTurnBasedSystemManager::OnRep_TurnOrder()
{
	AWarpGameState* GS = GetGameState();
	MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),  TEXT("Current turn order on client {%d}: "), GS->HasAuthority());
	for (int i = 0; i < TurnOrderUnitIds.Num(); i++)
	{
		MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("%d\n") , TurnOrderUnitIds[i]);
	}
	OnTurnOrderUpdated.Broadcast(TurnOrderUnitIds, CurrentTurnUnitId);
}