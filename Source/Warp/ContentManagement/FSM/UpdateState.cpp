// Fill out your copyright notice in the Description page of Project Settings.


#include "UpdateState.h"

void UUpdateState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	Super::OnEnter(InPrevState, InSwitchData);
}

bool UUpdateState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{
	return Super::OnExit(InNextState, InSwitchData);
}
