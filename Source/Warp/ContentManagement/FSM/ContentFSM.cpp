// Fill out your copyright notice in the Description page of Project Settings.


#include "ContentFSM.h"

#include "MGLogs.h"
#include "MGLogTypes.h"

DEFINE_LOG_CATEGORY_STATIC(ALogFSM, Log, All);

FString UContentFSMState::ToString() const
{
	return FString::Printf(TEXT("%s"), *GetFName().ToString());
}

bool UContentFSMState::IsEnterAccess(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	MG_LOG(ALogFSM,	TEXT("InPrevState %s; CurrentState %s"),
		InPrevState != nullptr ? *InPrevState->ToString() : TEXT("Null"), *ToString());

	return true;
}

void UContentFSMState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	MG_LOG(ALogFSM,	TEXT("InPrevState %s; CurrentState %s"), InPrevState != nullptr ? *InPrevState->ToString() : TEXT("Null"), *ToString());
	bActive = true;
}

bool UContentFSMState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{
	MG_LOG(ALogFSM,	TEXT("CurrentState %s; InNextState %s"), *ToString(), InNextState != nullptr ? *InNextState->ToString() : TEXT("Null"));

	bActive = false;
	return true;
}

UContentFSM::UContentFSM(): CurrentState(nullptr)
{
}


bool UContentFSM::Switch(UContentFSMState* InNewState, UContentFSMSwitchData* InSwitchData /*= nullptr*/)
{
	if(CurrentState == InNewState)
		return false;
	
	if (InNewState == nullptr || !InNewState->IsEnterAccess(CurrentState, InSwitchData))
		return false;
	
	if(CurrentState != nullptr && !CurrentState->OnExit(InNewState, InSwitchData))
		return false;

	MG_LOG(ALogFSM, TEXT("From: %s; -> To: %s"), CurrentState != nullptr ? *CurrentState->ToString() : TEXT("Null"), InNewState != nullptr ? *InNewState->ToString() : TEXT("Null"));

	UContentFSMState* OldState = CurrentState;
	CurrentState = InNewState;

	CurrentState->OnEnter(OldState, InSwitchData);

	return true;
}