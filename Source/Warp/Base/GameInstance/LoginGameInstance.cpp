// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginGameInstance.h"

#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "PlayFabRuntimeSettings.h"
#include "Warp/Base/GameInstanceSubsystem/WarpPlayfabContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(LoginGameInstanceLog, Log, All);

void ULoginGameInstance::Init()
{
	Super::Init();
	ClientAPI = IPlayFabModuleInterface::Get().GetClientAPI();

	LoginToPlayFab();
}

void ULoginGameInstance::LoginToPlayFab()
{
	RETURN_ON_FAIL_T(LogTemp, ClientAPI.IsValid(), TEXT("PlayFab ClientAPI is not valid in GameInstance"));

	using namespace PlayFab::ClientModels;

	FLoginWithCustomIDRequest Request;
	if (IsDedicatedServerInstance())
	{
		Request.CustomId = TEXT("DedicatedServer");
	}
	else
	{
		Request.CustomId = TEXT("DevClient");
	}
	Request.CreateAccount = true;

	ClientAPI->LoginWithCustomID(Request,
		PlayFab::UPlayFabClientAPI::FLoginWithCustomIDDelegate::CreateUObject(this, &ULoginGameInstance::OnLoginSuccess),
		PlayFab::FPlayFabErrorDelegate::CreateUObject(	this, &ULoginGameInstance::OnLoginError));
}

void ULoginGameInstance::OnLoginSuccess(const PlayFab::ClientModels::FLoginResult& Result)
{
	MG_LOG(LoginGameInstanceLog, TEXT("PlayFab login OK (GameInstance)"));
	
	if (auto* ContentSubsystem = GetSubsystem<UWarpPlayfabContentSubSystem>())
	{
		ContentSubsystem->DownloadUnits();
	}
}

void ULoginGameInstance::OnLoginError(const PlayFab::FPlayFabCppError& ErrorResult)
{
	MG_ERROR(LoginGameInstanceLog, TEXT("PlayFab login failed: %s"),
		*ErrorResult.GenerateErrorReport());
}