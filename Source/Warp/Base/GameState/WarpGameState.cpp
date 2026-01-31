// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpGameState.h"

#include "EngineUtils.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "Warp/Base/MatchStates.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
DEFINE_LOG_CATEGORY_STATIC(AWarpGameStateLog, Log, All);

AWarpGameState::AWarpGameState()
{
	bReplicateUsingRegisteredSubObjectList = true;
}

void AWarpGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

void AWarpGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	FDoRepLifetimeParams RepParams;
	RepParams.bIsPushBased = true;
	DOREPLIFETIME_WITH_PARAMS_FAST(AWarpGameState, CombatUnits_, RepParams);
}

void AWarpGameState::OnRep_MatchState()
{
	MG_LOG(AWarpGameStateLog, TEXT("MatchState: %s"), *MatchState.ToString());
	
	Super::OnRep_MatchState();
	
	if (MatchState == MatchState::Loading)
	{
		HandleMatchLoading();
	}
	
	// for (APlayerState* PS : PlayerArray)
	// {
	// 	if (PS)
	// 	{
	// 		if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(PS->GetPlayerController()))
	// 		{
	// 			PC->OnMatchStateChanged(MatchState);
	// 		}
	// 	}
	// }
}

void AWarpGameState::HandleMatchLoading()
{
	//NOTHING AWHILE
}

void AWarpGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
	
}

void AWarpGameState::HandleMatchHasStarted()
{
	Super::HandleMatchHasStarted();
	
}

void AWarpGameState::SetupCombatUnitsArray(const int InNumberOfUnits)
{
	CombatUnits_.Empty();
	CombatUnits_.Reserve(InNumberOfUnits);
}

void AWarpGameState::AddCombatUnit(ABaseUnitActor* InCombatUnit)
{
	RETURN_ON_FAIL(AWarpGameStateLog, InCombatUnit);
	CombatUnits_.Add(InCombatUnit);
}


void AWarpGameState::SendCombatUnitsToClients()
{
	// if (HasAuthority())
	// 	SetUnitsLoaded();
	// else
	MARK_PROPERTY_DIRTY_FROM_NAME(AWarpGameState, CombatUnits_, this);
}

void AWarpGameState::OnRep_CombatUnits()
{
	SetUnitsLoaded();
	MG_LOG(AWarpGameStateLog, TEXT("Replicated combat units; Num = %d"), CombatUnits_.Num());
}

void AWarpGameState::SetUnitsLoaded()
{
	for (APlayerState* PS : PlayerArray)
	{
		if (PS)
		{
			AWarpPlayerState* WPS = Cast<AWarpPlayerState>(PS);
			WPS->SetClientUnitsLoaded();
		}
	}
}
