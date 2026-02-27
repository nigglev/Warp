// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"
#include "ContentFSM.h"
#include "MGLogs.h"
#include "PlayFab.h"
#include "UObject/Object.h"
#include "SaveState.generated.h"

/**
 * 
 */


UCLASS()
class WARP_API USaveState : public UBaseState
{
	GENERATED_BODY()

public:
	virtual void OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData) override;
	virtual bool OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData) override;

protected:
	bool SaveDescriptionsBatchToPlayFab(const TMap<FName, FString>& InDescriptionsToSaveJson, const PlayFabServerPtr& InPlayFabAPI);
};
