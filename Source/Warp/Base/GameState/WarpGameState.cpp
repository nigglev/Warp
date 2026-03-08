// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpGameState.h"

#include "MGLogs.h"
#include "Engine/ActorChannel.h"
#include "GameFramework/PlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Warp/Base/MatchStates.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
#include "Warp/TurnBasedSystem/TurnMachine.h"

DEFINE_LOG_CATEGORY_STATIC(AWarpGameStateLog, Log, All);

AWarpGameState::AWarpGameState()
{
	bReplicateUsingRegisteredSubObjectList = true;
	
	TurnMachine_ = CreateDefaultSubobject<UTurnMachine>(TEXT("TurnMachine"));
}

void AWarpGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	
}

void AWarpGameState::BeginPlay()
{
	Super::BeginPlay();
}

void AWarpGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWarpGameState, TurnMachine_);
}

bool AWarpGameState::ReplicateSubobjects(UActorChannel* Channel, FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool bWroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);
	bWroteSomething |= Channel->ReplicateSubobject(TurnMachine_, *Bunch, *RepFlags);
	return bWroteSomething;
}

void AWarpGameState::OnRep_MatchState()
{
	MG_LOG(AWarpGameStateLog, TEXT("MatchState: %s"), *MatchState.ToString());
	
	if (MatchState == MatchState::UnitCreation)
	{
		HandleUnitCreation();
	}
	
	Super::OnRep_MatchState();
	
	OnMatchStateChanged.Broadcast(MatchState);
}

void AWarpGameState::HandleUnitCreation()
{
	TurnMachine_->CreateUnits();
	bUnitsCreated_ = true;
}

void AWarpGameState::HandleMatchIsWaitingToStart()
{
	Super::HandleMatchIsWaitingToStart();
}

void AWarpGameState::SetUnitsLoaded()
{
	MG_FUNC_LABEL(AWarpGameStateLog);
	
	for (APlayerState* PS : PlayerArray)
	{
		if (PS)
		{
			AWarpPlayerState* WPS = Cast<AWarpPlayerState>(PS);
			WPS->SetClientUnitsLoaded();
		}
	}
}
