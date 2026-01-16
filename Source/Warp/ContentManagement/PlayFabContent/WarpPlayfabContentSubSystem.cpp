// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayfabContentSubSystem.h"
#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Warp/ContentManagement/ContentManagementStates/States/PlayFabStateManager.h"
#include "Warp/Utils/WarpUtils.h"


DEFINE_LOG_CATEGORY_STATIC(ContentLog, Log, All);

UWarpPlayfabContentSubSystem::UWarpPlayfabContentSubSystem()
{
    //StateManager_ = CreateDefaultSubobject<UPlayFabStateManager>(TEXT("PFStateManager"));
    LoginInfo_ = CreateDefaultSubobject<UPlayFabLoginInfo>(TEXT("LoginInfo"));
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
    
    Descriptions_.Add(FUnitDescription::DescrName, MakeUnique<FUnitDescriptions>());
    
    bool bLoginAttemptSuccess = LoginToPlayFab();
    RETURN_ON_FAIL(ContentLog, bLoginAttemptSuccess)
}

void UWarpPlayfabContentSubSystem::OnLoginResult(const bool InLoginRes)
{
    MG_FUNC_LABEL(ContentLog);
    RETURN_ON_FAIL_T(ContentLog, InLoginRes, TEXT("Failed to Login"));
    if (!IsUEEditorActive())
    {
        StateManager_ = NewObject<UPlayFabStateManager>(this);
    }
    else
    {
        ReadDescriptions();
    }
}

bool UWarpPlayfabContentSubSystem::LoginToPlayFab()
{
    MG_COND_ERROR(ContentLog, LoginInfo_ != nullptr, TEXT("LoginInfo_ already exist"));
    LoginInfo_->OnLoginResult.AddUObject(this, &UWarpPlayfabContentSubSystem::OnLoginResult);
    ENetMode NetMode = GetWorld()->GetNetMode();
    bool bSuccess;
    TOptional<FString> SecretKey = WarpPlayfabContent::ReadSecret();
    if (SecretKey.IsSet() && NetMode != NM_Client)
    {
        UPlayFabRuntimeSettings* Settings = GetMutableDefault<UPlayFabRuntimeSettings>();
        RETURN_ON_FAIL_BOOL(ContentLog, Settings != nullptr);
        Settings->DeveloperSecretKey = SecretKey.GetValue();
        MG_LOG(ContentLog, TEXT("PlayFab secret set"));
        ServerAPI_ = IPlayFabModuleInterface::Get().GetServerAPI();
        MG_COND_ERROR(ContentLog, ServerAPI_ == nullptr, TEXT("Server API missing"));
        bSuccess = WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FServerTag>(ServerAPI_, LoginInfo_, TEXT("DedicatedServer"));
    }
    else
    {
        ClientAPI_ = IPlayFabModuleInterface::Get().GetClientAPI();
        MG_COND_ERROR(ContentLog, ClientAPI_ == nullptr, TEXT("Client API missing"));
        bSuccess = WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FClientTag>(ClientAPI_, LoginInfo_, TEXT("DevClient"));
    }

    return bSuccess;
}

bool UWarpPlayfabContentSubSystem::ReadDescriptions()
{
    const FString FolderDir = IsUEEditorActive()
        ? FPaths::Combine(FPaths::ProjectDir(), TEXT("GameDataSource"))
        : FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"));

    if (!IFileManager::Get().DirectoryExists(*FolderDir))
    {
        MG_ERROR(ContentLog, TEXT("GameDataSource directory does not exist: %s"), *FolderDir);
        return false;
    }

    for (auto It = Descriptions_.CreateIterator(); It; ++It)
    {
        const FName DescriptionName = It.Key();
        const FString FileName = DescriptionName.ToString() + TEXT(".json");
        const FString JsonPath = FPaths::Combine(FolderDir, FileName);

        FString JsonString;
        if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
        {
            MG_ERROR(ContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
            continue;
        }

        It.Value()->JsonToDescription(JsonString);
    }
    return true;
}


void UWarpPlayfabContentSubSystem::SaveDescriptionToPlayFab(const FString& InDescriptionName)
{
    RETURN_ON_FAIL(ContentLog, IsUEEditorActive());
    RETURN_ON_FAIL(ContentLog, !IsClient());
    RETURN_ON_FAIL(ContentLog, ServerAPI_);
    RETURN_ON_FAIL(ContentLog, !InDescriptionName.IsEmpty());
    
    // if (!Versions_)
    // {
    // 	Versions_ = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
    // }
    //
    // if (!CurrentDescriptionReader_)
    // {
    // 	CurrentDescriptionReader_ = CreateReaderByKey(FName(*InDescriptionName));
    // }
    //
    //
    // bool bOk = CurrentDescriptionReader_->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
    // RETURN_ON_FAIL_T(PFStateLog, bOk, TEXT("Failed to DescriptionReader from source"));
    // bOk = Versions_->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
    // RETURN_ON_FAIL_T(PFStateLog, bOk, TEXT("Failed to VersionsReader from source"));
    //
    // CurrentDescriptionReader_->SaveToPlayFab(ServerAPI_, this);
}

void UWarpPlayfabContentSubSystem::OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult)
{
    UE_LOG(LogTemp, Error, TEXT("PlayFab error: %s"),
           *ErrorResult.GenerateErrorReport());
}

bool UWarpPlayfabContentSubSystem::IsClient() const
{
    ENetMode NetMode = GetWorld()->GetNetMode();
    return  (NetMode == NM_Client);
}

void UWarpPlayfabContentSubSystem::BroadcastContentIsLoaded(bool InbIsContentLoaded)
{
    bUnitsLoaded_ = InbIsContentLoaded;
    OnUnitsLoaded.Broadcast();
}
