// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayfabContentSubSystem.h"

#include "HttpModule.h"
#include "MGLogs.h"
#include "PlayFabAuthenticationAPI.h"
#include "PlayFabServerAPI.h"
#include "PlayFabUtilities.h"
#include "Core/PlayFabClientAPI.h"
#include "Core/PlayFabServerAPI.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

DEFINE_LOG_CATEGORY_STATIC(ContentLog, Log, All);

namespace WarpPlayfabContent
{
    TOptional<FString> ReadSecret()
    {
        const FString PlayfabKeysPath(TEXT("PlayfabKeys"));
        const FString PathValue = FPlatformMisc::GetEnvironmentVariable(*PlayfabKeysPath);

        FString Secret;
        // Считать значение
        if (GConfig->GetString(TEXT("PlayFab"), TEXT("SecretKey"), Secret, PathValue))
        {
            Secret.TrimStartAndEndInline();
            Secret.ReplaceInline(TEXT("\""), TEXT(""));
            if (!Secret.IsEmpty())
            {
                return TOptional(std::move(Secret));
            }
        }
        
        return {};
    }
    
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
    void LoginWithCustomId(const TSharedPtr<typename TTag::TPlayFabAPI>& InPlayFabAPI, UWarpPlayfabContentSubSystem* InUserObject, const FString& InCustomId)
    {
        RETURN_ON_FAIL(ContentLog, InPlayFabAPI != nullptr);
        RETURN_ON_FAIL(ContentLog, InUserObject != nullptr);

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
        
            MG_LOG(ContentLog, TEXT("PlayFab login successful. PlayFabId_: %s; EntityToken_: %s; TokenExpiration_: %s; SessionTicket_: %s"), 
                *InUserObject->GetPlayFabId(), *InUserObject->GetEntityToken().Left(5), *InUserObject->GetEntityTokenExpiration().ToString(), 
                *InUserObject->GetSessionTicket().Left(5));
        });

        PlayFab::FPlayFabErrorDelegate ErrorDelegate;
        ErrorDelegate.BindWeakLambda(InUserObject, [](const PlayFab::FPlayFabCppError& InError)
        {
            MG_ERROR(ContentLog, TEXT("PlayFab login failed: %s"), *InError.GenerateErrorReport());
        });
    
        bool bLoginRes = InPlayFabAPI->LoginWithCustomID(Request, SuccessDelegate, ErrorDelegate);
        MG_COND_ERROR(ContentLog, !bLoginRes, TEXT("Login failed"));
    }
    
    // TValueOrError<FString, FString> StartAuthAndLoadUnits()
    // {
    //     TOptional<FString> Secret = ReadSecret();
    //     if (!Secret.IsSet())
    //     {
    //         return MakeError(TEXT("PlayFab secret missing."));
    //     }
	   //  
    //     TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    //     Req->SetURL(PFUrl(PlayFabTitleId, TEXT("Authentication/GetEntityToken")));
    //     Req->SetVerb(TEXT("POST"));
    //     Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    //     Req->SetHeader(TEXT("X-SecretKey"), Secret);
    //     Req->SetContentAsString(TEXT("{}"));
    //     Req->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
    //     {
    //         if (!bOk || !Resp.IsValid())
    //         {
    //             MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
    //             TEXT("GetEntityToken: no response"));
    //             return;
    //         }
    //         if (!EHttpResponseCodes::IsOk(Resp->GetResponseCode()))
    //         {
    //             MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
    //             TEXT("GetEntityToken: HTTP %d - %s"), Resp->GetResponseCode(), *Resp->GetContentAsString());
    //             return;
    //         }
    //
    //         FString EntityToken;
    //         {
    //             TSharedPtr<FJsonObject> J;
    //             FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Resp->GetContentAsString()), J);
    //             const TSharedPtr<FJsonObject>* Data = nullptr;
    //             if (J.IsValid() && J->TryGetObjectField(TEXT("data"), Data))
    //             {
    //                 (*Data)->TryGetStringField(TEXT("EntityToken"), EntityToken);
    //             }
    //         }
    //
    //         if (EntityToken.IsEmpty())
    //         {
    //             MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
    //             TEXT("GetEntityToken: missing token in response"));
    //             return;
    //         }
    //
    //         FetchUnitsFromCatalog(EntityToken);
    //     });
    //     Req->ProcessRequest();
    //}
}

UWarpPlayfabContentSubSystem* UWarpPlayfabContentSubSystem::Get(const UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject->GetWorld();
    RETURN_ON_FAIL_NULL(ContentLog, World);
    
    UGameInstance* GI = World->GetGameInstance();
    RETURN_ON_FAIL_NULL(ContentLog, GI);
    
    auto Content =	GI->GetSubsystem<UWarpPlayfabContentSubSystem>();
    RETURN_ON_FAIL_NULL(ContentLog, Content);

    return Content;
}

void UWarpPlayfabContentSubSystem::Initialize(FSubsystemCollectionBase& InCollection)
{
    Super::Initialize(InCollection);
    
    ENetMode NetMode = GetWorld()->GetNetMode();
    
    TOptional<FString> SecretKey = WarpPlayfabContent::ReadSecret();
    if (SecretKey.IsSet() && NetMode != NM_Client)
    {
        UPlayFabRuntimeSettings* Settings = GetMutableDefault<UPlayFabRuntimeSettings>();
        RETURN_ON_FAIL(ContentLog, Settings != nullptr);
        Settings->DeveloperSecretKey = SecretKey.GetValue();
        MG_LOG(ContentLog, TEXT("PlayFab secret set"));
        
        ServerAPI_ = IPlayFabModuleInterface::Get().GetServerAPI();
        MG_COND_ERROR(ContentLog, ServerAPI_ == nullptr, TEXT("Server API missing"));
        
        WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FServerTag>(ServerAPI_, this, TEXT("DedicatedServer"));
    }
    else
    {
        ClientAPI_ = IPlayFabModuleInterface::Get().GetClientAPI();
        MG_COND_ERROR(ContentLog, ClientAPI_ == nullptr, TEXT("Client API missing"));
    
        WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FClientTag>(ClientAPI_, this, TEXT("DevClient"));
    }
}

void UWarpPlayfabContentSubSystem::DownloadUnits()
{
    using namespace PlayFab::ClientModels;
    RETURN_ON_FAIL(ContentLog, ClientAPI_.IsValid())
    
    FGetTitleDataRequest Request;
    Request.Keys.Add(TEXT("Units"));

//    ServerAPI_->GetTitleData()
    ClientAPI_->GetTitleData(
        Request,
        PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate::CreateUObject(
            this, &UWarpPlayfabContentSubSystem::OnGetTitleDataSuccess),
        PlayFab::FPlayFabErrorDelegate::CreateUObject(
            this, &UWarpPlayfabContentSubSystem::OnPlayFabError));
}

void UWarpPlayfabContentSubSystem::OnGetTitleDataSuccess(
    const PlayFab::ClientModels::FGetTitleDataResult& Result)
{
    RETURN_ON_FAIL_T(ContentLog, !bUnitsLoaded_, TEXT("Already executed"));
    Units_.Empty();

    const FString* JsonStringPtr = Result.Data.Find(TEXT("Units"));
    RETURN_ON_FAIL_T(ContentLog, JsonStringPtr != nullptr, TEXT("TitleData key 'Units' not found"))


    TSharedPtr<FJsonObject> RootObject;
    TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(*JsonStringPtr);
    const bool bDeserialized = FJsonSerializer::Deserialize(Reader, RootObject);
    RETURN_ON_FAIL_T(ContentLog, bDeserialized, TEXT("Failed to parse Units JSON from TitleData"))

    const TArray<TSharedPtr<FJsonValue>>* UnitsArray = nullptr;
    const bool bGetArrayField = RootObject->TryGetArrayField(TEXT("units"), UnitsArray);
    RETURN_ON_FAIL_T(ContentLog, bGetArrayField, TEXT("JSON has no 'units' array"))

    for (const TSharedPtr<FJsonValue>& Value : *UnitsArray)
    {
        TSharedPtr<FJsonObject> UnitObj = Value->AsObject();
        if (!UnitObj.IsValid())
        {
            MG_ERROR(ContentLog, TEXT("Failed to get UnitObj"));
            continue;
        }

        FUnitDefinition Def;
        Def.UnitTypeName = FName(UnitObj->GetStringField(TEXT("UnitTypeName")));
        Def.UnitSize = UnitObj->GetStringField(TEXT("UnitSize"));
        Def.UnitSpeed = UnitObj->GetIntegerField(TEXT("UnitSpeed"));
        Def.UnitMaxAP = UnitObj->GetIntegerField(TEXT("UnitMaxAP"));

        Units_.Add(Def.UnitTypeName, Def);
    }
    bUnitsLoaded_ = true;
    UE_LOG(LogTemp, Log, TEXT("Loaded %d unit definitions from PlayFab"), Units_.Num());
    OnUnitsLoaded.Broadcast();
}

void UWarpPlayfabContentSubSystem::OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult)
{
    UE_LOG(LogTemp, Error, TEXT("PlayFab error: %s"),
           *ErrorResult.GenerateErrorReport());
}

const FUnitDefinition* UWarpPlayfabContentSubSystem::GetUnitDefinition(const FName& Id) const
{
    return Units_.Find(Id);
}
