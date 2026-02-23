// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ContentFSM.generated.h"

/**
 * 
 */
struct WARP_API UContentFSMSwitchData
{
	virtual ~UContentFSMSwitchData() {}
};


UCLASS()
class WARP_API UContentFSMState : public UObject
{
	GENERATED_BODY()

public:

	FString ToString() const;

	virtual bool IsEnterAccess(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData);
	virtual void OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData);
	virtual bool OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData);

	FORCEINLINE bool IsActive() const { return bActive; }

protected:

	UPROPERTY()
	bool bActive;
};

UCLASS()
class WARP_API UContentFSM : public UObject
{
	GENERATED_BODY()

public:	
	UContentFSM();

	bool Switch(UContentFSMState* InNewState, UContentFSMSwitchData* InSwitchData = nullptr);

	FORCEINLINE UContentFSMState* GetCurrentState() const { return CurrentState; }

private:

	UPROPERTY()
	UContentFSMState* CurrentState;
};
