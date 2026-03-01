// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginState.h"

#include "BaseState.h"
#include "SaveState.h"
#include "UpdateState.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(ALoginState, Log, All);

void ULoginState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	RETURN_ON_FAIL(ALoginState, InSwitchData);
	Super::OnEnter(InPrevState, InSwitchData);
	UWarpSwitchData* SwitchData = static_cast<UWarpSwitchData*>(InSwitchData);
	TSharedRef<UWarpSwitchData> SharedSwitchData = MakeShared<UWarpSwitchData>(*SwitchData);
	LoginToPlayFab(SharedSwitchData);
}

bool ULoginState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{	
	return Super::OnExit(InNextState, InSwitchData);
}


bool ULoginState::LoginToPlayFab(TSharedRef<UWarpSwitchData> InSwitchData)
{
	ERoleType Role = GetPlayfabContentSubsystem()->GetRoleType();
	bool bSuccess;
	TOptional<FString> SecretKey = ReadSecret();
	if (SecretKey.IsSet() && Role == ERoleType::Developer)
	{
		UPlayFabRuntimeSettings* Settings = GetMutableDefault<UPlayFabRuntimeSettings>();
		RETURN_ON_FAIL_BOOL(ALoginState, Settings != nullptr);
		Settings->DeveloperSecretKey = SecretKey.GetValue();
		ServerAPI_ = IPlayFabModuleInterface::Get().GetServerAPI();
		InSwitchData->ServerAPI = ServerAPI_;
		MG_COND_ERROR(ALoginState, ServerAPI_ == nullptr, TEXT("Server API missing"));
		bSuccess = LoginWithCustomId<FServerTag>(ServerAPI_, TEXT("DedicatedServer"), InSwitchData);
	
		MG_LOG(ALoginState, TEXT("PlayFab under secret set. bSuccess %d"), bSuccess);
	}
	else
	{
		ClientAPI_ = IPlayFabModuleInterface::Get().GetClientAPI();
		InSwitchData->ClientAPI = ClientAPI_;
		MG_COND_ERROR(ALoginState, ClientAPI_ == nullptr, TEXT("Client API missing"));
		bSuccess = LoginWithCustomId<FClientTag>(ClientAPI_, TEXT("DevClient"), InSwitchData);
        
		MG_LOG(ALoginState, TEXT("PlayFab simple. bSuccess %d"), bSuccess);
	}
	
	return bSuccess;
}

void ULoginState::OnLoginResult(bool InResult, TSharedRef<UWarpSwitchData> InSwitchData)
{
	if (InResult)
	{
		bool bIsSave = InSwitchData->DescriptionsToSaveJson.Num() > 0;
		if (bIsSave)
			SwitchToSaveState(InSwitchData);
		else
			SwitchToUpdateState(InSwitchData);
	}

	else
	{
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
	}
}

void ULoginState::SwitchToUpdateState(TSharedRef<UWarpSwitchData> InSwitchData)
{
	UContentFSM* FSM = GetPlayfabContentSubsystem()->GetContentFSM();
	RETURN_ON_FAIL(ALoginState, FSM);
	FSM->Switch(NewObject<UUpdateState>(FSM), &InSwitchData.Get());
}

void ULoginState::SwitchToSaveState(TSharedRef<UWarpSwitchData> InSwitchData)
{
	UContentFSM* FSM = GetPlayfabContentSubsystem()->GetContentFSM();
	RETURN_ON_FAIL(ALoginState, FSM);
	FSM->Switch(NewObject<USaveState>(FSM), &InSwitchData.Get());
}

TOptional<FString> ULoginState::ReadSecret()
{
	const FString PlayfabKeysPath(TEXT("PlayfabKeys"));
	const FString PathValue = FPlatformMisc::GetEnvironmentVariable(*PlayfabKeysPath);

	FString Secret;
	if (GConfig->GetString(TEXT("PlayFab"), TEXT("SecretKey"), Secret, PathValue))
	{
		Secret.TrimStartAndEndInline();
		Secret.ReplaceInline(TEXT("\""), TEXT(""));

		if (!Secret.IsEmpty())
		{
			return TOptional<FString>(MoveTemp(Secret)); // UE-style move
		}
	}

	return {};
}
