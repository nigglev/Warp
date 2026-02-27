// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseState.h"
#include "ContentFSM.h"
#include "PlayFab.h"
#include "PlayFabClientDataModels.h"
#include "PlayFabServerDataModels.h"
#include "Core/PlayFabClientAPI.h"
#include "Core/PlayFabServerAPI.h"
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
	void SwitchToUpdateState();
	void SwitchToSaveState();

	struct FServerTag
	{
		using TPlayFabAPI = PlayFab::UPlayFabServerAPI;
		using TLoginWithCustomIDRequest = PlayFab::ServerModels::FLoginWithCustomIDRequest;
		using TLoginWithCustomIDResult = PlayFab::ServerModels::FServerLoginResult;
		using TLoginWithCustomIDDelegate = PlayFab::UPlayFabServerAPI::FLoginWithCustomIDDelegate;
	};

	struct FClientTag
	{
		using TPlayFabAPI = PlayFab::UPlayFabClientAPI;
		using TLoginWithCustomIDRequest = PlayFab::ClientModels::FLoginWithCustomIDRequest;
		using TLoginWithCustomIDResult = PlayFab::ClientModels::FLoginResult;
		using TLoginWithCustomIDDelegate = PlayFab::UPlayFabClientAPI::FLoginWithCustomIDDelegate;
	};

	template<typename TTag>
    bool LoginWithCustomId(const TSharedPtr<typename TTag::TPlayFabAPI>& InPlayFabAPI, const FString& InCustomId)
    {
		ensure(InPlayFabAPI != nullptr);

        typename TTag::TLoginWithCustomIDRequest Request;
        Request.CustomId = InCustomId;
        Request.CreateAccount = true;

        typename TTag::TLoginWithCustomIDDelegate SuccessDelegate;
        SuccessDelegate.BindWeakLambda(this, [this](const typename TTag::TLoginWithCustomIDResult& InResult)
        {
            PlayFabId_ = InResult.PlayFabId;
            if (InResult.EntityToken.IsValid())
            {
            	EntityToken_ = InResult.EntityToken->EntityToken;
            	EntityTokenExpiration_ = InResult.EntityToken->TokenExpiration;
            }

            SessionTicket_ = InResult.SessionTicket;
        	this->OnLoginResult(true);

            // MG_LOG(WarpPlayfabContentLog, TEXT("PlayFab login successful. PlayFabId_: %s; EntityToken_: %s; TokenExpiration_: %s; SessionTicket_: %s"),
            //     *InUserObject->GetPlayFabId(),
            //     *InUserObject->GetEntityToken().Left(5),
            //     *InUserObject->GetEntityTokenExpiration().ToString(),
            //     *InUserObject->GetSessionTicket().Left(5));
        });

        PlayFab::FPlayFabErrorDelegate ErrorDelegate;
        ErrorDelegate.BindWeakLambda(this, [this](const PlayFab::FPlayFabCppError& InError)
        {
        	this->OnLoginResult(false);
           // MG_ERROR(WarpPlayfabContentLog, TEXT("PlayFab login failed: %s"), *InError.GenerateErrorReport());
        });

        const bool bLoginRes = InPlayFabAPI->LoginWithCustomID(Request, SuccessDelegate, ErrorDelegate);
       // MG_COND_ERROR(WarpPlayfabContentLog, !bLoginRes, TEXT("Login failed"));
        return bLoginRes;
    }
	
	PlayFabClientPtr ClientAPI_ = nullptr;
	PlayFabServerPtr ServerAPI_ = nullptr;
	
	FString PlayFabId_;
	FString EntityToken_;
	FDateTime EntityTokenExpiration_;
	FString SessionTicket_;
};
