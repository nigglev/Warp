// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnMachine.h"

#include "MGLogs.h"
#include "Algo/AllOf.h"
#include "Math/UnitConversion.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Actors/UnitActors/UnitActorFactory.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/ContentManagement/PlayFabContent/WarpContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(ATurnMachineLog, Log, All);

AWarpGameState* UTurnMachine::GetOwner() const
{
    return Cast<AWarpGameState>(GetOuter());
}

FString FTurnState::ToString() const
{
    if (IsValid(ActiveUnit))
        return FString::Printf(TEXT("Phase: %s; Active Unit: %s"), 
             *StaticEnum<ETurnPhase>()->GetNameStringByValue(static_cast<int64>(Phase)),
             *ActiveUnit->GetDebugName());
    return FString::Printf(TEXT("Phase: %s"), *StaticEnum<ETurnPhase>()->GetNameStringByValue(static_cast<int64>(Phase)));
}

void UTurnMachine::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    UObject::GetLifetimeReplicatedProps(OutLifetimeProps);
    
    FDoRepLifetimeParams RepParams;
    RepParams.bIsPushBased = true;
    DOREPLIFETIME_WITH_PARAMS_FAST(UTurnMachine, CombatUnits_, RepParams);
    DOREPLIFETIME_WITH_PARAMS_FAST(UTurnMachine, TurnState_, RepParams);
    DOREPLIFETIME(UTurnMachine, RoundIndex_);
}

void UTurnMachine::CreateUnits()
{
    RETURN_ON_FAIL(ATurnMachineLog, GetOwner());
    
    GetOwner()->OnUnitArrived.AddUObject(this, &UTurnMachine::OnUnitArrived);
    
    const FGameplayDescription* Descr = UWarpContentSubSystem::GetGameplayDescription(this);
    RETURN_ON_FAIL(ATurnMachineLog, Descr);
	
    for(int32 i = 0; i < Descr->ShipsOnStart; i++)
    {
        HexMath::FAxialCoord AC(i*4, i * 2);
        int32 Ind = FMath::RandRange(0, Descr->DefaultPlayerUnitTypes.Num() - 1);
        ABaseUnitActor* Unit = UnitActorFactory::CreateUnitActor(this, Descr->DefaultPlayerUnitTypes[Ind], FAxialTransform(AC, {}));
        if (Unit != nullptr)
        {
            CombatUnits_.Add(Unit);
        }
    }
    
    SortUnits();

    FLaunchContext Context = BuildLaunchContext(this);
    if (IsServerLike(Context))
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(UTurnMachine, CombatUnits_, this);
    }
    
    if (!CombatUnits_.IsEmpty())
    {
        SetNewActiveUnit();
    }
    
    if (IsServerClientMix(Context))
    {
        OnRep_CombatUnits();
    }
}

void UTurnMachine::OnRep_CombatUnits()
{
    MG_LOG(ATurnMachineLog, TEXT("Replicated combat units; Num = %d"), CombatUnits_.Num());
    CheckLoaded();
    //GetOwner()->OnTurnOrderChanged.Broadcast(CombatUnits_, TurnState_.ActiveUnitIndex, TurnState_.RoundNumber);
}

void UTurnMachine::CheckLoaded()
{
    RETURN_ON_FAIL(ATurnMachineLog, GetOwner());
    if (IsValidState())
    {
        GetOwner()->SetUnitsLoaded();
    }
    else
    {
        GetWorld()->GetTimerManager().SetTimerForNextTick(FTimerDelegate::CreateUObject(this, &UTurnMachine::CheckLoaded));
    }
}

void UTurnMachine::SetNewActiveUnit()
{
    RETURN_ON_FAIL(ATurnMachineLog, !CombatUnits_.IsEmpty());
    
    TurnState_.ActiveUnit = CombatUnits_.Last();
    TurnState_.Phase = ETurnPhase::WaitingForInput;
    
    MG_LOG(ATurnMachineLog, TEXT("TurnState_: %s"), *TurnState_.ToString());
    
    FLaunchContext Context = BuildLaunchContext(this);
    if (IsServerLike(Context))
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(UTurnMachine, TurnState_, this);
    }
    
    if (IsServerClientMix(Context))
    {
        OnRep_TurnState();
    }
}

void UTurnMachine::OnRep_TurnState()
{
    MG_LOG(ATurnMachineLog, TEXT("TurnState: %s"), *TurnState_.ToString());
    CheckLoaded();
    
    ABaseUnitActor* NewActiveUnit = GetActiveUnit();
    RETURN_ON_FAIL(ATurnMachineLog, NewActiveUnit);
    RETURN_ON_FAIL(ATurnMachineLog, NewActiveUnit->IsLoaded());
    
    if (TurnState_.Phase == ETurnPhase::WaitingForInput)
    {
        GetOwner()->OnUnitSelected.Broadcast(NewActiveUnit, PrevActiveUnit_);
        PrevActiveUnit_ = NewActiveUnit;
    }
    else
    {
        GetOwner()->OnUnitStartMoving.Broadcast(NewActiveUnit);
        //GetOwner()->OnTurnOrderChanged.Broadcast(CombatUnits_, TurnState_.ActiveUnitIndex, TurnState_.RoundNumber);
    }
    
}

void UTurnMachine::SetWaitingForArrival()
{
    TurnState_.Phase = ETurnPhase::WaitingForArrival;
    
    MG_LOG(ATurnMachineLog, TEXT("TurnState_: %s"), *TurnState_.ToString());
    
    FLaunchContext Context = BuildLaunchContext(this);
    if (IsServerLike(Context))
    {
        MARK_PROPERTY_DIRTY_FROM_NAME(UTurnMachine, TurnState_, this);
    }
    
    if (IsServerClientMix(Context))
    {
        OnRep_TurnState();
    }
}

bool UTurnMachine::IsValidState() const
{
    if (!IsValid(TurnState_.ActiveUnit))
        return false;
    
    if (CombatUnits_.IsEmpty())
        return false;
    
    bool AllLoaded = Algo::AllOf(CombatUnits_, [](const ABaseUnitActor* Unit) { return Unit && Unit->IsLoaded(); });
    
    return AllLoaded;
}

const ABaseUnitActor* UTurnMachine::GetActiveUnit() const
{
    RETURN_ON_FAIL_NULL(ATurnMachineLog, GetOwner());
    //RETURN_ON_FAIL_NULL(ATurnMachineLog, CombatUnits_.IsValidIndex(TurnState_.ActiveUnitIndex));
    if (CombatUnits_.IsEmpty())
        return nullptr;
    
    const ABaseUnitActor* Unit = CombatUnits_.Last();
    RETURN_ON_FAIL_NULL(ATurnMachineLog, IsValid(Unit));
    
    return Unit;
}

ABaseUnitActor* UTurnMachine::GetActiveUnit()
{
    return const_cast<ABaseUnitActor*>(static_cast<const UTurnMachine*>(this)->GetActiveUnit());
}

// int32 UTurnMachine::GetActiveUnitIndex() const
// {
//     return TurnState_.ActiveUnitIndex;
// }
//
// void UTurnMachine::BroadcastTurnOrderInfo() const
// {
//     GetOwner()->OnTurnOrderChanged.Broadcast(CombatUnits_, TurnState_.ActiveUnitIndex, TurnState_.RoundNumber);
// }

bool UTurnMachine::CanAcceptMove() const
{
    if (!IsValidState())
        return false;

    return TurnState_.Phase == ETurnPhase::WaitingForInput;
}

void UTurnMachine::RequestMove(const FAxialTransform& InTarget)
{
    MG_LOG(ATurnMachineLog, TEXT("InTarget: %s"), *InTarget.ToString());
    
    if (!CanAcceptMove())
        return;
    
    FLaunchContext Context = BuildLaunchContext(this);
    if (!IsAuthorityLike(Context))
        return;

    ABaseUnitActor* Active = GetActiveUnit();
    if (!IsValid(Active))
        return;

    if (Active->SetMoveTarget(InTarget))
        SetWaitingForArrival();
}

void UTurnMachine::OnUnitArrived(ABaseUnitActor* InUnit)
{
    RETURN_ON_FAIL(ATurnMachineLog, InUnit);
    RETURN_ON_FAIL(ATurnMachineLog, InUnit == GetActiveUnit());
    
    MG_LOG(ATurnMachineLog, TEXT("InUnit: %s"), *InUnit->GetDebugName());
    
    NextRoundStore_.Add(InUnit);
    CombatUnits_.Remove(InUnit);
    
    if (CombatUnits_.IsEmpty())
        NewRound();
    
    SetNewActiveUnit();
}

void UTurnMachine::NewRound()
{
    CombatUnits_ = MoveTemp(NextRoundStore_);
    
    SortUnits();
    
    RoundIndex_++;
    MG_LOG(ATurnMachineLog, TEXT("New round: %u"), RoundIndex_);

    for (ABaseUnitActor* Unit : CombatUnits_)
    {
        Unit->OnNewRound(RoundIndex_);
    }
    
    FLaunchContext Context = BuildLaunchContext(this);
    if (IsServerClientMix(Context))
    {
        OnRep_RoundIndex();
    }
}

void UTurnMachine::SortUnits()
{
    CombatUnits_.Sort([](const ABaseUnitActor& Unit1, const ABaseUnitActor& Unit2)
    {
        if (Unit1.GetMovePriority() != Unit2.GetMovePriority())
        {
            return Unit1.GetMovePriority() < Unit2.GetMovePriority();
        }

        return Unit1.GetUniqueID() < Unit2.GetUniqueID();
    });  
}

void UTurnMachine::OnRep_RoundIndex()
{
    MG_LOG(ATurnMachineLog, TEXT("RoundIndex: %u"), RoundIndex_);
    GetOwner()->OnNewRound.Broadcast(RoundIndex_);   
}
