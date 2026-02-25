// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayfabContentSubSystem.h"
#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Warp/ContentManagement/ContentManagementStates/States/PlayFabStateManager.h"
#include "Warp/ContentManagement/FSM/ContentFSM.h"
#include "Warp/ContentManagement/FSM/LoginState.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameplayDescriptions.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"

DEFINE_LOG_CATEGORY_STATIC(AContentLog, Log, All);

UWarpPlayfabContentSubSystem::UWarpPlayfabContentSubSystem()
{   
    ContentFSM_ = CreateDefaultSubobject<UContentFSM>(TEXT("ContentFSM"));
}

UWarpPlayfabContentSubSystem* UWarpPlayfabContentSubSystem::Get(const UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject->GetWorld();
    RETURN_ON_FAIL_NULL(AContentLog, World);
    
    UGameInstance* GI = World->GetGameInstance();
    RETURN_ON_FAIL_NULL(AContentLog, GI);
    
    auto Content =	GI->GetSubsystem<UWarpPlayfabContentSubSystem>();
    RETURN_ON_FAIL_NULL(AContentLog, Content);

    return Content;
}

void UWarpPlayfabContentSubSystem::Initialize(FSubsystemCollectionBase& InCollection)
{
    Super::Initialize(InCollection);

    //RoleType_ = GetCurrentRoleType();
    RoleType_ = ERoleType::Server;
    InitializeDescriptions();
    bool bSuccess = ReadDescriptionsFromDataSource();
    RETURN_ON_FAIL(AContentLog, bSuccess);
    
    if (RoleType_ == ERoleType::Developer)
    {
        OnContentCheckedAndLoaded(true);
    }
    if (RoleType_ == ERoleType::Client || RoleType_ == ERoleType::Server)
    {
        UpdateCachedGameData();
    }
}

void UWarpPlayfabContentSubSystem::InitializeDescriptions()
{
    Descriptions_.Add(FGameplayDescription::DescrName, MakeUnique<FGameplayDescriptions>());
    Descriptions_.Add(FUnitDescription::DescrName, MakeUnique<FUnitDescriptions>());
}

FDescriptionVersions UWarpPlayfabContentSubSystem::GetGameVersionFromDataSource()
{
    FDescriptionVersions Versions;
    const FString FolderDir = GetGameDataSourceFilePath();
    if (FolderDir.IsEmpty())
    {
        MG_ERROR(AContentLog, TEXT("Folder with game version does not exist"));
        return Versions;
    }
        
    const FString JsonPath = FPaths::Combine(FolderDir, VersionsFileName);

    if (!FPaths::FileExists(JsonPath))
    {
        MG_WARNING(AContentLog, TEXT("Game version file does not exist"));
        return Versions;
    }
    
    FString JsonString;
    FText* FailReason = nullptr;
    if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
    {
        MG_ERROR(AContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
        return Versions;
    }
    
    bool bOk = Versions.JsonToVersions(JsonString, FailReason);
    if (!bOk)
    {
        MG_ERROR(AContentLog, TEXT("Failed to convert Json to versions: %s"), *FailReason->ToString());
        return Versions;
    }
    return Versions;
}

bool UWarpPlayfabContentSubSystem::ReadDescriptionsFromDataSource()
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(AContentLog, !FolderDir.IsEmpty());
        
    for (TTuple<FName, TUniquePtr<FBaseDescriptions>>& Descriptions : Descriptions_)
    {
        const FName DescriptionName = Descriptions.Get<0>();
        const FString FileName = DescriptionName.ToString() + TEXT(".json");
        const FString JsonPath = FPaths::Combine(FolderDir, FileName);

        if (!FPaths::FileExists(JsonPath))
        {
            FString JsonString;
            Descriptions.Get<1>()->EmplaceNewItem();
            bool bSuccess = Descriptions.Get<1>()->DescriptionToJson(JsonString);
            WriteDescriptionToDataSourceFromJson(DescriptionName.ToString(), JsonString);
            MG_COND_ERROR(AContentLog, !bSuccess, TEXT("Failed to convert description to json"));
            continue;
        }
   

        FString JsonString;
        FText* FailReason = nullptr;
        if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
        {
            MG_ERROR(AContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
            continue;
        }
        
        bool bSuccess = Descriptions.Get<1>()->JsonToDescription(JsonString, FailReason);
        MG_COND_ERROR(AContentLog, !bSuccess, TEXT("Failed to convert Json to description: %s"), *FailReason->ToString());
        MG_COND_LOG(AContentLog, bSuccess, TEXT("Description has been read: %s"), *DescriptionName.ToString());
    }
    
    return true;
}


bool UWarpPlayfabContentSubSystem::WriteGameVersionToDataSource(const FDescriptionVersions& InGameVersion)
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(AContentLog, !FolderDir.IsEmpty());
    const FString DescriptionJsonPath = FPaths::Combine(FolderDir, VersionsFileName);;

    FString JsonString;
    const bool bOk = InGameVersion.VersionsToJson(JsonString);
    RETURN_ON_FAIL_BOOL_T(AContentLog, bOk, TEXT("Failed to JSON convert"));

    if (!FFileHelper::SaveStringToFile(JsonString, *DescriptionJsonPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        MG_ERROR(DescriptionReaderLog, TEXT("Failed to write to file: %s"), *DescriptionJsonPath);
        return false;
    }

    return true;
}

bool UWarpPlayfabContentSubSystem::WriteDescriptionToDataSourceFromJson(const FString& InDescriptionName,
                                                                        const FString& InDescriptionJson)
{   
    FBaseDescriptions* Descriptions = Descriptions_.Find(FName(*InDescriptionName))->Get();
    RETURN_ON_FAIL_BOOL_T(AContentLog, Descriptions, TEXT("Description not found: %s"), *InDescriptionName);
    FText* FailReason = nullptr;
    bool bOk = Descriptions->JsonToDescription(InDescriptionJson, FailReason);
    RETURN_ON_FAIL_BOOL_T(AContentLog, bOk, TEXT("Failed to convert Json to description: %s"), *FailReason->ToString());
    bOk = WriteDescriptionToDataSource(FName(*InDescriptionName));
    RETURN_ON_FAIL_BOOL_T(AContentLog, bOk, TEXT("Failed to write description to data source"));

    return true;
}

bool UWarpPlayfabContentSubSystem::WriteDescriptionToDataSource_Internal(const FName& DescriptionName, FBaseDescriptions& Description)
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(AContentLog, !FolderDir.IsEmpty());
    const FString FileName = DescriptionName.ToString() + TEXT(".json");
    const FString DescriptionJsonPath = FPaths::Combine(FolderDir, FileName);

    FString JsonString;
    const bool bOk = Description.DescriptionToJson(JsonString);
    RETURN_ON_FAIL_BOOL_T(AContentLog, bOk, TEXT("Failed to JSON convert"));

    if (!FFileHelper::SaveStringToFile(JsonString, *DescriptionJsonPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        MG_ERROR(DescriptionReaderLog, TEXT("Failed to write to file: %s"), *DescriptionJsonPath);
        return false;
    }

    return true;
}

bool UWarpPlayfabContentSubSystem::WriteDescriptionsToDataSource()
{
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
    RETURN_ON_FAIL_BOOL(AContentLog, InDescriptionName.IsValid());
    
    if (InDescriptionName.IsNone())
    {
        WriteDescriptionsToDataSource();
        return true;
    }

    TUniquePtr<FBaseDescriptions>* Found = Descriptions_.Find(InDescriptionName);
    RETURN_ON_FAIL_BOOL_T(AContentLog, Found, TEXT("Description not found: %s"), *InDescriptionName.ToString());
    
    if (Found->Get()->AreItemsEmpty())
        Found->Get()->EmplaceNewItem();
    
    return WriteDescriptionToDataSource_Internal(InDescriptionName, *Found->Get());
}


// bool UWarpPlayfabContentSubSystem::SaveDescriptionToPlayFab(const FName& InDescriptionName)
// {
//     RETURN_ON_FAIL_BOOL(AContentLog, LaunchContext_.bIsPIE);
//     RETURN_ON_FAIL_BOOL(AContentLog, GetClientEnv(LaunchContext_) == EClientEnv::NotAClient);
//     RETURN_ON_FAIL_BOOL(AContentLog, ServerAPI_);
//     RETURN_ON_FAIL_BOOL(AContentLog, InDescriptionName.IsValid());
//
//     const FString FolderDir = GetGameDataSourceFilePath();
//     RETURN_ON_FAIL_BOOL(AContentLog, !FolderDir.IsEmpty());
//     const FName DescriptionName = InDescriptionName;
//     const FString FileName = DescriptionName.ToString() + TEXT(".json");
//     const FString JsonPath = FPaths::Combine(FolderDir, FileName);
//     
//     FString JsonString;
//     if (!FFileHelper::LoadFileToString(JsonString, *JsonPath))
//     {
//         MG_ERROR(AContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
//         return false;
//     }
//     
//     bool bSaveRes = WarpPlayfabContent::SaveDescriptionToPlayFab(ServerAPI_, InDescriptionName.ToString(), JsonString, this);
//     return bSaveRes;
// }
//
// void UWarpPlayfabContentSubSystem::OnDescriptionSavingResult(bool bSucceeded)
// {
//     if (bSucceeded && !bSaveDescriptionToPlayFabDone_)
//     {
//         MG_LOG(AContentLog, TEXT("Successful description save"));
//         bSaveDescriptionToPlayFabDone_ = true;
//         
//     } else if (bSucceeded && bSaveDescriptionToPlayFabDone_)
//     {
//         MG_LOG(AContentLog, TEXT("Successful save"));
//         bSaveVersionToPlayFabDone_ = true;
//     }
//     else
//     {
//         MG_ERROR(AContentLog, TEXT("Failed to save"));
//     }
// }

FString UWarpPlayfabContentSubSystem::GetGameDataSourceFilePath() const
{   
    FString FolderDir;
    if (RoleType_ == ERoleType::NotSet)
    {
        FolderDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"));
        MG_ERROR(AContentLog, TEXT("RoleType_ is not set; Reading from cache"));
    }
    if (RoleType_ == ERoleType::Developer)
    {
        FolderDir = FPaths::Combine(FPaths::ProjectDir(), TEXT("GameDataSource"));
    }
    else if (RoleType_ == ERoleType::Client || RoleType_ == ERoleType::Server)
    {
        FolderDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"));
    }

    if (!IFileManager::Get().DirectoryExists(*FolderDir))
    {
        MG_ERROR(AContentLog, TEXT("GameDataSource directory does not exist: %s"), *FolderDir);
        return FString();
    }

    return FolderDir;
    
}

bool UWarpPlayfabContentSubSystem::UpdateCachedGameData()
{
    ContentFSM_->Switch(NewObject<ULoginState>(ContentFSM_), nullptr);
    return true;   
}

ERoleType UWarpPlayfabContentSubSystem::GetCurrentRoleType() const
{
    FLaunchContext LaunchContext = BuildLaunchContext(GetWorld());
    MG_LOG(AContentLog, TEXT("%s"), *LaunchContext.ToString());
    
    if (LaunchContext.bIsPIE)
    {
        return ERoleType::Developer;
    }
    if (LaunchContext.NetMode == ELaunchNetMode::Client)
    {
        return ERoleType::Client;
    }
    return ERoleType::Server;
}

void UWarpPlayfabContentSubSystem::OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult)
{
    UE_LOG(LogTemp, Error, TEXT("PlayFab error: %s"),
           *ErrorResult.GenerateErrorReport());
}


void UWarpPlayfabContentSubSystem::OnContentCheckedAndLoaded(bool InContentLoaded)
{
    MG_COND_WARNING(AContentLog, !InContentLoaded, TEXT("Failed to load content"));
    MG_LOG(AContentLog, TEXT("InContentLoaded: %d"), InContentLoaded);

    if (ContentFSM_)
        ContentFSM_ = nullptr;
    
    bContentLoaded_ = true;
    OnContentLoaded.Broadcast();
}
