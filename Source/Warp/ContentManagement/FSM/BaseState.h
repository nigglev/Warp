// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ContentFSM.h"
#include "PlayFab.h"
#include "UObject/Object.h"
#include "BaseState.generated.h"


class UWarpPlayfabContentSubSystem;
class UPlayFabLoginInfo;

struct WARP_API UWarpSwitchData : public UContentFSMSwitchData
{
	UPlayFabLoginInfo* LoginInfo = nullptr;
	PlayFabClientPtr ClientAPI = nullptr;
	PlayFabServerPtr ServerAPI = nullptr;
	
	virtual ~UWarpSwitchData() override
	{
		LoginInfo = nullptr;
		ClientAPI = nullptr;
		ServerAPI = nullptr;
	};
};

/**
 * 
 */
UCLASS()
class WARP_API UBaseState : public UContentFSMState
{
	GENERATED_BODY()

public:

	virtual void OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData) override;
	virtual bool OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData) override;

protected:
	virtual UWarpPlayfabContentSubSystem* GetPlayfabContentSubsystem() const;
};
