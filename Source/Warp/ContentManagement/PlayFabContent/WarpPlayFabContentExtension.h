#pragma once

#include "CoreMinimal.h"
#include "PlayFabClientDataModels.h"
#include "Core/PlayFabClientAPI.h"
#include "PlayFabServerDataModels.h"
#include "Core/PlayFabServerAPI.h"
#include "MGLogs.h"
#include "PlayFab.h"
#include "WarpPlayFabContentExtension.generated.h"

struct FGameVersion;
class UWarpPlayfabContentSubSystem;
class UPlayFabStateManager;
DEFINE_LOG_CATEGORY_STATIC(WarpPlayfabContentLog, Log, All);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnLoginResult, bool /*bLoginRes*/)

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

    FOnLoginResult OnLoginResult;

protected:

    FString PlayFabId_;
    FString EntityToken_;
    FDateTime EntityTokenExpiration_;
    FString SessionTicket_;

};

class FDescriptionReaderBase;

namespace WarpPlayfabContent
{
    using FReaderFactory = TFunction<TUniquePtr<FDescriptionReaderBase>()>;
    TUniquePtr<FDescriptionReaderBase> CreateReaderByKey(const FName& Key);
    
    TOptional<FString> ReadSecret();
    bool SaveDescriptionToPlayFab(const PlayFabServerPtr& InPlayFabAPI, const FString& InKey, const FString& InJsonToSave, UWarpPlayfabContentSubSystem* InContentSubSystem);
    bool SaveVersionsToPlayFab(const PlayFabServerPtr& InPlayFabAPI);

    bool DownloadVersionsFromPlayFab(const PlayFabClientPtr& InPlayFabAPI, FGameVersion& OutVersions);
    
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
    
}
