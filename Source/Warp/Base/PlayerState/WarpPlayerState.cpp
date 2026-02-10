// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayerState.h"

#include "MGLogs.h"

DEFINE_LOG_CATEGORY_STATIC(WarpPlayerStateLog, Log, All);

void AWarpPlayerState::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

void AWarpPlayerState::SetClientLoaded()
{
	MG_LOG(WarpPlayerStateLog, TEXT("%s"), *GetName());
	bClientLoaded = true;
	MsgToServerClientLoaded();
}

void AWarpPlayerState::SetClientUnitsLoaded()
{
	MG_LOG(WarpPlayerStateLog, TEXT("%s"), *GetName());
	bIsClientUnitsLoaded = true;
	MsgToServerClientUnitsLoaded();
}

void AWarpPlayerState::MsgToServerClientLoaded_Implementation()
{
	MG_LOG(WarpPlayerStateLog, TEXT("%s"), *GetName());
	bClientLoaded = true;	
}

void AWarpPlayerState::MsgToServerClientUnitsLoaded_Implementation()
{
	MG_LOG(WarpPlayerStateLog, TEXT("%s"), *GetName());
	bIsClientUnitsLoaded = true;	
}
