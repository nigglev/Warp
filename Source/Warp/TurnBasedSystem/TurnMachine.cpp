// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnMachine.h"

#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Base/GameState/WarpGameState.h"

void UTurnMachine::Initialize(AWarpGameState* InGameState)
{
    GameState_ = InGameState;
}

UWorld* UTurnMachine::GetWorld() const
{
    return GameState_.IsValid() ? GameState_->GetWorld() : nullptr;
}

UWorld* UTurnMachine::GetTickableGameObjectWorld() const
{
    return GetWorld();
}

TStatId UTurnMachine::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(UTurnMachine, STATGROUP_Tickables);
}

bool UTurnMachine::IsTickable() const
{
    // Only server drives progression; clients don't need to tick.
    if (!bRunning_ || !GameState_.IsValid() || !GameState_->HasAuthority())
        return false;

    // We only need tick while waiting for the active unit to arrive.
    return GameState_->GetTurnPhase() == ETurnPhase::WaitingForArrival;
}

void UTurnMachine::Start()
{
    bRunning_ = true;
    RefreshUnitsFromGameState();

    // Server initializes turn state if needed
    if (GameState_.IsValid() && GameState_->HasAuthority())
    {
        if (GameState_->GetCombatUnits().Num() > 0 && GameState_->GetActiveUnitIndex() == INDEX_NONE)
        {
            GameState_->SetActiveUnitIndex(0);
            GameState_->SetTurnPhase(ETurnPhase::WaitingForInput);
            GameState_->ForceNetUpdate();
        }
    }
}

void UTurnMachine::RefreshUnitsFromGameState()
{
    Units_.Reset();

    if (!GameState_.IsValid())
        return;

    Units_.Reserve(GameState_->GetCombatUnits().Num());
    for (ABaseUnitActor* U : GameState_->GetCombatUnits())
    {
        Units_.Add(U);
    }

    if (GameState_->HasAuthority())
    {
        if (Units_.Num() == 0)
        {
            GameState_->SetActiveUnitIndex(INDEX_NONE);
            GameState_->SetTurnPhase(ETurnPhase::WaitingForInput);
            GameState_->ForceNetUpdate();
        }
        else if (!GameState_->GetCombatUnits().IsValidIndex(GameState_->GetActiveUnitIndex()))
        {
            GameState_->SetActiveUnitIndex(0);
            GameState_->SetTurnPhase(ETurnPhase::WaitingForInput);
            GameState_->ForceNetUpdate();
        }
    }
}

void UTurnMachine::OnTurnStateReplicated()
{
}

bool UTurnMachine::CanAcceptMove() const
{
    if (!GameState_.IsValid())
        return false;

    return GameState_->GetTurnPhase() == ETurnPhase::WaitingForInput
        && GameState_->GetCombatUnits().Num() > 0
        && GameState_->GetActiveUnitIndex() != INDEX_NONE;
}

ABaseUnitActor* UTurnMachine::GetServerActiveUnit() const
{
    if (!GameState_.IsValid())
        return nullptr;

    return GameState_->GetActiveUnit();
}

bool UTurnMachine::RequestMove(const FRepAxialCoord& InTarget)
{
    if (!GameState_.IsValid() || !GameState_->HasAuthority())
        return false;

    if (!CanAcceptMove())
        return false;

    ABaseUnitActor* Active = GetServerActiveUnit();
    if (!IsValid(Active))
        return false;

    Active->SetMoveTarget(InTarget);

    GameState_->SetTurnPhase(ETurnPhase::WaitingForArrival);
    GameState_->ForceNetUpdate();
    return true;
}

void UTurnMachine::Tick(float DeltaTime)
{
    if (!GameState_.IsValid() || !GameState_->HasAuthority())
        return;
    
    if (GameState_->GetTurnPhase() != ETurnPhase::WaitingForArrival)
        return;

    ABaseUnitActor* Active = GetServerActiveUnit();
    if (!IsValid(Active))
    {
        ServerAdvanceTurn();
        return;
    }

    if (!Active->IsMoving())
    {
        ServerAdvanceTurn();
    }
}

void UTurnMachine::ServerAdvanceTurn()
{
    if (!GameState_.IsValid() || !GameState_->HasAuthority())
        return;

    const int32 Num = GameState_->GetCombatUnits().Num();
    if (Num <= 0)
    {
        GameState_->SetActiveUnitIndex(INDEX_NONE);
        GameState_->SetTurnPhase(ETurnPhase::WaitingForInput);
        GameState_->ForceNetUpdate();
        return;
    }

    int32 Next = GameState_->GetActiveUnitIndex();
    if (Next == INDEX_NONE) Next = 0;

    for (int32 Try = 0; Try < Num; ++Try)
    {
        Next = (Next + 1) % Num;
        if (IsValid(GameState_->GetCombatUnits()[Next]))
        {
            GameState_->SetActiveUnitIndex(Next);
            GameState_->SetTurnPhase(ETurnPhase::WaitingForInput);
            GameState_->ForceNetUpdate();
            return;
        }
    }

    GameState_->SetActiveUnitIndex(INDEX_NONE);
    GameState_->SetTurnPhase(ETurnPhase::WaitingForInput);
    GameState_->ForceNetUpdate();
}
