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

void UTurnBasedSystemManager::StartTurns(const int32 Seed)
{
	if (!GetGameState() || !GetGameState()->HasAuthority() || bTurnsStarted)
		return;
	TurnSeed = Seed;
	BuildTurnOrder();

	if (TurnOrderUnitIds.Num() == 0)
	{
		MG_COND_ERROR(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
		TEXT("StartTurns: no units."));
		return;
	}

	bTurnsStarted = true;
	TurnIt = 0;
	BeginTurnFor(TurnOrderUnitIds[TurnIt]);
}

void UTurnBasedSystemManager::BuildTurnOrder()
{
	if (!GetGameState() || !GetGameState()->HasAuthority())
		return;
	
	SortTurnOrder();
	OnTurnOrderUpdated.Broadcast(TurnOrderUnitIds, CurrentTurnUnitId);
	
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
		TurnIt = NewIdx;
	}
	else
	{
		TurnIt = 0;
		CurrentTurnUnitId = TurnOrderUnitIds.Num() ? TurnOrderUnitIds[0] : 0;
		MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, CurrentTurnUnitId, this);
		
	}
	OnTurnOrderUpdated.Broadcast(TurnOrderUnitIds, CurrentTurnUnitId);
}


void UTurnBasedSystemManager::BeginTurnFor(uint32 UnitId)
{
	if (!GetGameState() || !GetGameState()->HasAuthority())
		return;
	
	CurrentTurnUnitId = UnitId;
	MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, CurrentTurnUnitId, this);
	Phase = ETurnPhase::InTurn;
	MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, Phase, this);
	
	OnTurnPhaseChanged.Broadcast(Phase);
	MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::GameState),  TEXT("Starting turn for unit id = %d") , CurrentTurnUnitId);
	
	if (UUnitBase* U = GetCurrentTurnUnit())
	{
		U->StartNewTurn();
		//ProcessAITurn(U);
	}
}

void UTurnBasedSystemManager::AdvanceToNextUnit()
{
	if (!GetGameState() || !GetGameState()->HasAuthority() || TurnOrderUnitIds.Num() == 0) return;
	TurnIt = (TurnIt + 1) % TurnOrderUnitIds.Num();
	BeginTurnFor(TurnOrderUnitIds[TurnIt]);
}

void UTurnBasedSystemManager::SortTurnOrder()
{
	if (!GetGameState() || !GetGameState()->HasAuthority())
		return;
	
	TArray<UUnitBase*> Copy = GetGameState()->GetActiveUnits();
	Copy.RemoveAll([](const UUnitBase* U){ return U == nullptr; });

	Copy.Sort([&](const UUnitBase& A, const UUnitBase& B)
	{
		if (A.GetSpeed() != B.GetSpeed()) return A.GetSpeed() > B.GetSpeed();
		return GetTieBreak(A.GetUnitID()) < GetTieBreak(B.GetUnitID()); 
	});

	TurnOrderUnitIds.Reset(Copy.Num());
	for (const UUnitBase* U : Copy)
		TurnOrderUnitIds.Add(U->GetUnitID());
	MARK_PROPERTY_DIRTY_FROM_NAME(UTurnBasedSystemManager, TurnOrderUnitIds, this);
}

int32 UTurnBasedSystemManager::GetTieBreak(uint32 UnitId) const
{
	const uint32 Seed = HashCombine(TurnSeed, ::GetTypeHash(UnitId));
	FRandomStream Rng(Seed);
	return static_cast<int32>(Rng.GetUnsignedInt());
}

UUnitBase* UTurnBasedSystemManager::GetCurrentTurnUnit() const
{
	if (!GetGameState())
		return nullptr;
	
	for (UUnitBase* U : GetGameState()->GetActiveUnits())
	{
		if (U && U->GetUnitID() == CurrentTurnUnitId) return U;
	}
	return nullptr;
}


// void AWarpGameState::ProcessAITurn(UUnitBase* Unit)
// {
// 	if (!Unit) return;
//
// 	const bool bPlayer = (Unit->GetAffiliation() == EUnitAffiliation::PlayerControlled);
// 	if (bPlayer)
// 		return;
//
// 	while (Unit->HasAnyAffordableAction(MinActionPointCost))
// 	{
// 		Unit->SpendAP(MinActionPointCost);
// 	}
// 	AdvanceToNextUnit();
// }

AWarpGameState* UTurnBasedSystemManager::GetGameState() const
{
	return GetTypedOuter<AWarpGameState>();
}


void UTurnBasedSystemManager::OnRep_CurrentTurn()
{
	MG_COND_LOG(UTurnBasedSystemManagerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::TurnBasedSystemManager),
	TEXT("CurrentTurn %d"),	CurrentTurnUnitId);
	OnTurnOrderUpdated.Broadcast(TurnOrderUnitIds, CurrentTurnUnitId);
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