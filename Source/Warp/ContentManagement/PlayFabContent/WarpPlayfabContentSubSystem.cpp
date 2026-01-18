// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayfabContentSubSystem.h"
#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Warp/ContentManagement/ContentManagementStates/States/PlayFabStateManager.h"

DEFINE_LOG_CATEGORY_STATIC(ContentLog, Log, All);

UWarpPlayfabContentSubSystem::UWarpPlayfabContentSubSystem()
{   
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

    if (IsServerOnly() || IsServerEditor())
    {
        BroadcastContentIsLoaded(true);
    }
    if (IsClientEditor())
    {
        StateManager_ = NewObject<UPlayFabStateManager>(this);
        StateManager_->SetClientAPI(ClientAPI_);
        StateManager_->Start();
    }
    if (IsClientOnly())
    {
        StateManager_ = NewObject<UPlayFabStateManager>(this);
        StateManager_->SetClientAPI(ClientAPI_);
        StateManager_->Start();
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

bool UWarpPlayfabContentSubSystem::ReadDescriptionsFromDataSource()
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(ContentLog, !FolderDir.IsEmpty());
        
    for (TTuple<FName, TUniquePtr<FBaseDescriptions>>& Descriptions : Descriptions_)
    {
        const FName DescriptionName = Descriptions.Get<0>();
        const FString FileName = DescriptionName.ToString() + TEXT(".json");
        const FString JsonPath = FPaths::Combine(FolderDir, FileName);

        FString JsonString;
        FText* FailReason = nullptr;
        if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
        {
            MG_ERROR(ContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
            continue;
        }

        bool bSuccess = Descriptions.Get<1>()->JsonToDescription(JsonString, FailReason);
        if (!bSuccess)
        {
            MG_ERROR(ContentLog, TEXT("Failed to convert Json to description: %s"), *FailReason->ToString());
            continue;
        }
    }
    return true;
}

bool UWarpPlayfabContentSubSystem::WriteDescriptionToDataSource_Internal(const FName& DescriptionName, FBaseDescriptions& Description)
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(ContentLog, !FolderDir.IsEmpty());
    const FString FileName = DescriptionName.ToString() + TEXT(".json");
    const FString DescriptionJsonPath = FPaths::Combine(FolderDir, FileName);

    FString JsonString;
    const bool bOk = Description.DescriptionToJson(JsonString);
    RETURN_ON_FAIL_BOOL_T(ContentLog, bOk, TEXT("Failed to JSON convert"));

    if (!FFileHelper::SaveStringToFile(JsonString, *DescriptionJsonPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        MG_ERROR(DescriptionReaderLog, TEXT("Failed to write to file: %s"), *DescriptionJsonPath);
        return false;
    }

    return true;
}

bool UWarpPlayfabContentSubSystem::WriteDescriptionsToDataSource()
{
    RETURN_ON_FAIL_BOOL(ContentLog, IsUEEditorActive());
    RETURN_ON_FAIL_BOOL(ContentLog, !IsClient());

    bool bAllOk = true;

    for (TTuple<FName, TUniquePtr<FBaseDescriptions>>& DescriptionPair : Descriptions_)
    {
        const FName DescriptionName = DescriptionPair.Get<0>();
        FBaseDescriptions* Description = DescriptionPair.Get<1>().Get();

        if (!ensure(Description))
        {
            bAllOk = false;
            continue;
        }

        if (!WriteDescriptionToDataSource_Internal(DescriptionName, *Description))
        {
            bAllOk = false;
        }
    }

    return bAllOk;
}

bool UWarpPlayfabContentSubSystem::WriteDescriptionToDataSource(const FName& InDescriptionName)
{
    RETURN_ON_FAIL_BOOL(ContentLog, IsUEEditorActive());
    RETURN_ON_FAIL_BOOL(ContentLog, !IsClient());
    RETURN_ON_FAIL_BOOL(ContentLog, InDescriptionName.IsValid());

    TUniquePtr<FBaseDescriptions>* Found = Descriptions_.Find(InDescriptionName);
    RETURN_ON_FAIL_BOOL(ContentLog, Found);

    return WriteDescriptionToDataSource_Internal(InDescriptionName, *Found->Get());
}

bool UWarpPlayfabContentSubSystem::SaveDescriptionToPlayFab(const FName& InDescriptionName)
{
    RETURN_ON_FAIL_BOOL(ContentLog, IsUEEditorActive());
    RETURN_ON_FAIL_BOOL(ContentLog, !IsClient());
    RETURN_ON_FAIL_BOOL(ContentLog, ServerAPI_);
    RETURN_ON_FAIL_BOOL(ContentLog, InDescriptionName.IsValid());

    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(ContentLog, !FolderDir.IsEmpty());
    const FName DescriptionName = InDescriptionName;
    const FString FileName = DescriptionName.ToString() + TEXT(".json");
    const FString JsonPath = FPaths::Combine(FolderDir, FileName);
    
    FString JsonString;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
    {
        MG_ERROR(ContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
        return false;
    }
    
    bool bSaveRes = WarpPlayfabContent::SaveDescriptionToPlayFab(ServerAPI_, InDescriptionName.ToString(), JsonString, this);
    return bSaveRes;
}

void UWarpPlayfabContentSubSystem::OnDescriptionSavingResult(bool bSucceeded)
{
    if (bSucceeded && !bSaveDescriptionToPlayFabDone_)
    {
        MG_LOG(ContentLog, TEXT("Successful description save"));
        bSaveDescriptionToPlayFabDone_ = true;
        SaveVersionsToPlayFab();
    } else if (bSucceeded && bSaveDescriptionToPlayFabDone_)
    {
        MG_LOG(ContentLog, TEXT("Successful save"));
        bSaveVersionToPlayFabDone_ = true;
    }
    else
    {
        MG_ERROR(ContentLog, TEXT("Failed to save"));
    }
}

FDescriptionVersions UWarpPlayfabContentSubSystem::CreateVersions()
{
    if (Descriptions_.Num() == 0)
        return FDescriptionVersions();
    
    FDescriptionVersions Versions;
    for (TTuple<FName, TUniquePtr<FBaseDescriptions>>& DescriptionPair : Descriptions_)
    {
        const FName DescriptionName = DescriptionPair.Get<0>();
        const FBaseDescriptions* Description = DescriptionPair.Get<1>().Get();

        if (!DescriptionName.IsValid() || Description == nullptr)
        {
            MG_ERROR(ContentLog, TEXT("Failed to get %s"), *DescriptionName.ToString());
            continue;
        }

        FDescriptionVersion VersionInfo;
        VersionInfo.DescriptionName = DescriptionName.ToString();
        VersionInfo.Version = Description->Version;

        Versions.Items.Add(VersionInfo);
    }
    Versions.UpdateVersion();
    return Versions;
}

bool UWarpPlayfabContentSubSystem::WriteVersionsToDataSource(const FDescriptionVersions& InVersions, FString& OutJsonString)
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(ContentLog, !FolderDir.IsEmpty());
    const FString DescriptionJsonPath = FPaths::Combine(FolderDir, VersionsFileName);
    
    const bool bOk = InVersions.VersionsToJson(OutJsonString);
    RETURN_ON_FAIL_BOOL_T(ContentLog, bOk, TEXT("Failed to JSON convert"));

    if (!FFileHelper::SaveStringToFile(OutJsonString, *DescriptionJsonPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        MG_ERROR(DescriptionReaderLog, TEXT("Failed to write to file: %s"), *DescriptionJsonPath);
        return false;
    }

    return true;
}

bool UWarpPlayfabContentSubSystem::SaveVersionsToPlayFab()
{
    RETURN_ON_FAIL_BOOL(ContentLog, IsUEEditorActive());
    RETURN_ON_FAIL_BOOL(ContentLog, !IsClient());
    RETURN_ON_FAIL_BOOL(ContentLog, ServerAPI_);

    FDescriptionVersions VersionsToSave = CreateVersions();
    FString JsonString;
    bool bOk = WriteVersionsToDataSource(VersionsToSave, JsonString);
    RETURN_ON_FAIL_BOOL_T(ContentLog, bOk, TEXT("Failed to write to file"));
    bool bSaveRes = WarpPlayfabContent::SaveDescriptionToPlayFab(ServerAPI_, FString("DescriptionVersions"), JsonString, this);
    return bSaveRes;
}


FString UWarpPlayfabContentSubSystem::GetGameDataSourceFilePath() const
{
    FString FolderDir;
    if (IsServerOnly() || IsServerEditor())
    {
        FolderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("GameDataSource"));
    }
    if (IsClientEditor())
    {
        FolderDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"));
    }
    if (IsClientOnly())
    {
        FolderDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"));
    }
    
    if (!IFileManager::Get().DirectoryExists(*FolderDir))
    {
        MG_ERROR(ContentLog, TEXT("GameDataSource directory does not exist: %s"), *FolderDir);
        return FString();
    }

    return FolderDir;
    
}

void UWarpPlayfabContentSubSystem::OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult)
{
    UE_LOG(LogTemp, Error, TEXT("PlayFab error: %s"),
           *ErrorResult.GenerateErrorReport());
}

bool UWarpPlayfabContentSubSystem::IsClientOnly() const
{
    return !IsUEEditorActive() && IsClient();
}

bool UWarpPlayfabContentSubSystem::IsClientEditor() const
{
    return IsUEEditorActive() && IsClient();
}

bool UWarpPlayfabContentSubSystem::IsServerOnly() const
{
    return !IsUEEditorActive() && !IsClient();
}

bool UWarpPlayfabContentSubSystem::IsServerEditor() const
{
    return IsUEEditorActive() && !IsClient();
}

bool UWarpPlayfabContentSubSystem::IsClient() const
{
    ENetMode NetMode = GetWorld()->GetNetMode();
    return  (NetMode == NM_Client);
}

void UWarpPlayfabContentSubSystem::BroadcastContentIsLoaded(bool InbIsContentLoaded)
{
    RETURN_ON_FAIL_T(ContentLog, InbIsContentLoaded, TEXT("Failed to load content"));
    
    if (StateManager_)
        StateManager_ = nullptr;
    
    if (InbIsContentLoaded)
    {
        bool bSuccess = ReadDescriptionsFromDataSource();
        RETURN_ON_FAIL(ContentLog, bSuccess);
        bUnitsLoaded_ = true;
        OnUnitsLoaded.Broadcast();
    }
}
