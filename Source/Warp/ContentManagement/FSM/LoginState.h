// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"
#include "ContentFSM.h"
#include "PlayFab.h"
#include "UObject/Object.h"
#include "LoginState.generated.h"

class UWarpPlayfabContentSubSystem;
class UPlayFabLoginInfo;
/**
 * 
 */

UCLASS()
class WARP_API ULoginState : public UBaseState
{
	GENERATED_BODY()

public:

	virtual void OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData) override;
	virtual bool OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData) override;

protected:
	bool LoginToPlayFab();
	void OnLoginResult(bool InResult);

	UPROPERTY()
	UPlayFabLoginInfo* LoginInfo_ = nullptr;
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;
};
