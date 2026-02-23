// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"
#include "ContentFSM.h"
#include "UObject/Object.h"
#include "UpdateState.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API UUpdateState : public UBaseState
{
	GENERATED_BODY()

public:

	virtual void OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData) override;
	virtual bool OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData) override;
};
