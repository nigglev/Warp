// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PlayFab.h"
#include "Core/PlayFabError.h"
#include "Core/PlayFabClientDataModels.h"
#include "LoginGameInstance.generated.h"

namespace PlayFab
{
	struct FPlayFabCppError;
}

namespace PlayFab::ClientModels
{
	struct FLoginResult;
}

/**
 * 
 */
UCLASS()
class WARP_API ULoginGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
	virtual void Init() override;

private:
	PlayFabClientPtr ClientAPI = nullptr;

	void LoginToPlayFab();
	
	void OnLoginSuccess(const PlayFab::ClientModels::FLoginResult& Result);
	void OnLoginError(const PlayFab::FPlayFabCppError& ErrorResult);
};
