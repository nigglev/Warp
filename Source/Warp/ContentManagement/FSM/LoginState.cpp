// Fill out your copyright notice in the Description page of Project Settings.


#include "LoginState.h"

#include "BaseState.h"
#include "UpdateState.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayFabContentExtension.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"

DEFINE_LOG_CATEGORY_STATIC(ALoginState, Log, All);

void ULoginState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	Super::OnEnter(InPrevState, InSwitchData);
	LoginInfo_ = NewObject<UPlayFabLoginInfo>();
	LoginToPlayFab();
}

bool ULoginState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{	
	return Super::OnExit(InNextState, InSwitchData);
}


bool ULoginState::LoginToPlayFab()
{
	LoginInfo_->OnLoginResult.AddUObject(this, &ULoginState::OnLoginResult);
	ERoleType Role = GetPlayfabContentSubsystem()->GetRoleType();
	bool bSuccess;
	TOptional<FString> SecretKey = WarpPlayfabContent::ReadSecret();
	if (SecretKey.IsSet() && Role == ERoleType::Developer)
	{
		UPlayFabRuntimeSettings* Settings = GetMutableDefault<UPlayFabRuntimeSettings>();
		RETURN_ON_FAIL_BOOL(ALoginState, Settings != nullptr);
		Settings->DeveloperSecretKey = SecretKey.GetValue();
		ServerAPI_ = IPlayFabModuleInterface::Get().GetServerAPI();
		MG_COND_ERROR(ALoginState, ServerAPI_ == nullptr, TEXT("Server API missing"));
		bSuccess = WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FServerTag>(ServerAPI_, LoginInfo_, TEXT("DedicatedServer"));
	
		MG_LOG(ALoginState, TEXT("PlayFab under secret set. bSuccess %d"), bSuccess);
	}
	else
	{
		ClientAPI_ = IPlayFabModuleInterface::Get().GetClientAPI();
		MG_COND_ERROR(ALoginState, ClientAPI_ == nullptr, TEXT("Client API missing"));
		bSuccess = WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FClientTag>(ClientAPI_, LoginInfo_, TEXT("DevClient"));
        
		MG_LOG(ALoginState, TEXT("PlayFab simple. bSuccess %d"), bSuccess);
	}
	
	return bSuccess;
}

void ULoginState::OnLoginResult(bool InResult)
{
	if (InResult)
	{
		RETURN_ON_FAIL(ALoginState, GetWorld());
		UContentFSM* FSM = GetPlayfabContentSubsystem()->GetContentFSM();
		RETURN_ON_FAIL(ALoginState, FSM);
		UWarpSwitchData WarpSwitchData;
		WarpSwitchData.LoginInfo = LoginInfo_;
		WarpSwitchData.ServerAPI = ServerAPI_;
		WarpSwitchData.ClientAPI = ClientAPI_;
		FSM->Switch(NewObject<UUpdateState>(), &WarpSwitchData);
		
	}

	else
	{
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
	}
}
