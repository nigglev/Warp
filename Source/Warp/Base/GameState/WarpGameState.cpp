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
#include "Warp/CombatMap(Deprecated)/CombatMap.h"
#include "Warp/TurnBasedSystem(Deprecated)/Manager/TurnBasedSystemManager.h"
#include "Warp/Units(Deprecated)/UnitBase.h"
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
}

void AWarpGameState::OnRep_MatchState()
{
	MG_LOG(AWarpGameStateLog, TEXT("MatchState: %s"), *MatchState.ToString());
	
	Super::OnRep_MatchState();
	
	if (MatchState == MatchState::Loading)
	{
		HandleMatchLoading();
	}
	
	//Player Controller Iteration
	for (APlayerState* PS : PlayerArray)
	{
		if (PS)
		{
			if (ADefaultPlayerController* PC = Cast<ADefaultPlayerController>(PS->GetPlayerController()))
			{
				PC->OnMatchStateChanged(MatchState);
			}
		}
	}
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

