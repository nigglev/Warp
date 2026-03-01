// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseState.h"

#include "MGLogs.h"
#include "Warp/ContentManagement/PlayFabContent/WarpContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(ABaseState, Log, All);

void UBaseState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	Super::OnEnter(InPrevState, InSwitchData);
}

bool UBaseState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{
	return Super::OnExit(InNextState, InSwitchData);
}

UWarpContentSubSystem* UBaseState::GetPlayfabContentSubsystem() const
{
	RETURN_ON_FAIL_NULL(ABaseState, GetWorld());
	UWarpContentSubSystem* SubSystem = UWarpContentSubSystem::Get(GetWorld());
	RETURN_ON_FAIL_NULL(ABaseState, SubSystem);
	return SubSystem;
}
