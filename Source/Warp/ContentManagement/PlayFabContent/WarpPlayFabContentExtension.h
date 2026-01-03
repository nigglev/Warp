#pragma once

#include "CoreMinimal.h"

#include "PlayFabClientDataModels.h"
#include "Core/PlayFabClientAPI.h"
#include "PlayFabServerDataModels.h"
#include "Core/PlayFabServerAPI.h"
#include "MGLogs.h"
#include "WarpPlayFabContentExtension.generated.h"

class UPlayFabStateManager;
DEFINE_LOG_CATEGORY_STATIC(WarpPlayfabContentLog, Log, All);

UCLASS()
class UPlayFabLoginInfo : public UObject
{
    GENERATED_BODY()
public:
    void SetPlayFabId(const FString& InPlayFabId) { PlayFabId_ = InPlayFabId; }
    void SetEntityToken(const FString& InEntityToken, const FDateTime& InExpiration) { EntityToken_ = InEntityToken; EntityTokenExpiration_ = InExpiration; }
    void SetSessionTicket(const FString& InSessionTicket) { SessionTicket_ = InSessionTicket; }
	
    FString GetPlayFabId() const { return PlayFabId_; }
    FString GetEntityToken() const { return EntityToken_; }
    FDateTime GetEntityTokenExpiration() const { return EntityTokenExpiration_; }
    FString GetSessionTicket() const { return SessionTicket_; }

protected:
    FString PlayFabId_;
    FString EntityToken_;
    FDateTime EntityTokenExpiration_;
    FString SessionTicket_;

};

namespace WarpPlayfabContent
{
    TOptional<FString> ReadSecret();

    struct FServerTag
    {
        using TPlayFabAPI = PlayFab::UPlayFabServerAPI;
        using TLoginWithCustomIDRequest = PlayFab::ServerModels::FLoginWithCustomIDRequest;
        using TLoginWithCustomIDResult = PlayFab::ServerModels::FServerLoginResult;
        using TLoginWithCustomIDDelegate = PlayFab::UPlayFabServerAPI::FLoginWithCustomIDDelegate;

        using TGetTitleDataRequest  = PlayFab::ServerModels::FGetTitleDataRequest;
        using TGetTitleDataResult   = PlayFab::ServerModels::FGetTitleDataResult;
        using TGetTitleDataDelegate = PlayFab::UPlayFabServerAPI::FGetTitleDataDelegate;
    };

    struct FClientTag
    {
        using TPlayFabAPI = PlayFab::UPlayFabClientAPI;
        using TLoginWithCustomIDRequest = PlayFab::ClientModels::FLoginWithCustomIDRequest;
        using TLoginWithCustomIDResult = PlayFab::ClientModels::FLoginResult;
        using TLoginWithCustomIDDelegate = PlayFab::UPlayFabClientAPI::FLoginWithCustomIDDelegate;

        using TGetTitleDataRequest  = PlayFab::ClientModels::FGetTitleDataRequest;
        using TGetTitleDataResult   = PlayFab::ClientModels::FGetTitleDataResult;
        using TGetTitleDataDelegate = PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate;
    };

    template<typename TTag>
    bool LoginWithCustomId(
        const TSharedPtr<typename TTag::TPlayFabAPI>& InPlayFabAPI,
        UPlayFabLoginInfo* InUserObject,
        const FString& InCustomId)
    {
        RETURN_ON_FAIL_BOOL(WarpPlayfabContentLog, InPlayFabAPI != nullptr);
        RETURN_ON_FAIL_BOOL(WarpPlayfabContentLog, InUserObject != nullptr);

        typename TTag::TLoginWithCustomIDRequest Request;
        Request.CustomId = InCustomId;
        Request.CreateAccount = true;

        typename TTag::TLoginWithCustomIDDelegate SuccessDelegate;
        SuccessDelegate.BindWeakLambda(InUserObject, [InUserObject](const typename TTag::TLoginWithCustomIDResult& InResult)
        {
            InUserObject->SetPlayFabId(InResult.PlayFabId);

            if (InResult.EntityToken.IsValid())
            {
                InUserObject->SetEntityToken(InResult.EntityToken->EntityToken, InResult.EntityToken->TokenExpiration.mValue);
            }

            InUserObject->SetSessionTicket(InResult.SessionTicket);

            MG_LOG(WarpPlayfabContentLog, TEXT("PlayFab login successful. PlayFabId_: %s; EntityToken_: %s; TokenExpiration_: %s; SessionTicket_: %s"),
                *InUserObject->GetPlayFabId(),
                *InUserObject->GetEntityToken().Left(5),
                *InUserObject->GetEntityTokenExpiration().ToString(),
                *InUserObject->GetSessionTicket().Left(5));
        });

        PlayFab::FPlayFabErrorDelegate ErrorDelegate;
        ErrorDelegate.BindWeakLambda(InUserObject, [](const PlayFab::FPlayFabCppError& InError)
        {
            MG_ERROR(WarpPlayfabContentLog, TEXT("PlayFab login failed: %s"), *InError.GenerateErrorReport());
        });

        const bool bLoginRes = InPlayFabAPI->LoginWithCustomID(Request, SuccessDelegate, ErrorDelegate);
        MG_COND_ERROR(WarpPlayfabContentLog, !bLoginRes, TEXT("Login failed"));
        return bLoginRes;
    }
}
