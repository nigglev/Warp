// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"
#include "ContentFSM.h"
#include "PlayFabClientDataModels.h"
#include "PlayFabError.h"
#include "UObject/Object.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameVersion.h"
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

protected:
	bool DownloadVersions();
	void OnGetGameVersionTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result);
	void OnGetGameVersionTitleDataError(const PlayFab::FPlayFabCppError& ErrorResult);

	void OnGetContentTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result);
	void OnGetContentTitleDataError(const PlayFab::FPlayFabCppError& ErrorResult);

	TArray<FString> GetContentToUpdate(const TArray<FDescriptionVersion>& InCurrentDescriptions, const TArray<FDescriptionVersion>& InPlayFabDescriptions);
	bool UpdateContent(const TArray<FString>& InContentToUpdate);

	PlayFabClientPtr ClientAPI_ = nullptr;
	FGameVersion PlayFabVersion_;
};
