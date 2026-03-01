// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpContentSubSystem.h"
#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Warp/ContentManagement/FSM/ContentFSM.h"
#include "Warp/ContentManagement/FSM/LoginState.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameplayDescriptions.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"

DEFINE_LOG_CATEGORY_STATIC(AContentLog, Log, All);

UWarpContentSubSystem::UWarpContentSubSystem()
{   
   
}

UWarpContentSubSystem* UWarpContentSubSystem::Get(const UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject->GetWorld();
    RETURN_ON_FAIL_NULL(AContentLog, World);
    
    UGameInstance* GI = World->GetGameInstance();
    RETURN_ON_FAIL_NULL(AContentLog, GI);
    
    auto Content =	GI->GetSubsystem<UWarpContentSubSystem>();
    RETURN_ON_FAIL_NULL(AContentLog, Content);

    return Content;
}

const FGameplayDescription* UWarpContentSubSystem::GetGameplayDescription(const UObject* WorldContextObject)
{
    UWarpContentSubSystem* PlayfabContentSubSystem = Get(WorldContextObject);
    RETURN_ON_FAIL_NULL(AContentLog, PlayfabContentSubSystem);
	
    const FGameplayDescription* Descr = PlayfabContentSubSystem->GetFirstDescription<FGameplayDescription>();
    RETURN_ON_FAIL_NULL(AContentLog, Descr);
    return Descr;
}

const FUnitDescription* UWarpContentSubSystem::GetUnitDescription(const UObject* WorldContextObject, FName InUnitType)
{
    UWarpContentSubSystem* PlayfabContentSubSystem = Get(WorldContextObject);
    RETURN_ON_FAIL_NULL(AContentLog, PlayfabContentSubSystem);
	
    const FUnitDescription* Descr = PlayfabContentSubSystem->GetDescription<FUnitDescription>(InUnitType);
    RETURN_ON_FAIL_NULL(AContentLog, Descr);
    return Descr;
}

void UWarpContentSubSystem::Initialize(FSubsystemCollectionBase& InCollection)
{
    Super::Initialize(InCollection);

    RoleType_ = GetCurrentRoleType();

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

void UWarpContentSubSystem::InitializeDescriptions()
{
    Descriptions_.Add(FGameplayDescription::DescrName, MakeUnique<FGameplayDescriptions>());
    Descriptions_.Add(FUnitDescription::DescrName, MakeUnique<FUnitDescriptions>());
}

FGameVersion UWarpContentSubSystem::GetGameVersionFromDataSource()
{
    FGameVersion Versions;
    const FString FolderDir = GetGameDataSourceFilePath();
    if (FolderDir.IsEmpty())
    {
        MG_ERROR(AContentLog, TEXT("Folder with game version does not exist"));
        return Versions;
    }

    const FString FileName = FGameVersion::Name.ToString() + TEXT(".json");
    const FString JsonPath = FPaths::Combine(FolderDir, FileName);

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

bool UWarpContentSubSystem::ReadDescriptionsFromDataSource()
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
        
        const TCHAR* ReasonStr = FailReason != nullptr ? *FailReason->ToString() : TEXT("Unknown");
        MG_COND_ERROR(AContentLog, !bSuccess, TEXT("Failed to convert Json to description: %s"), ReasonStr);
        MG_COND_LOG(AContentLog, bSuccess, TEXT("Description has been read: %s"), *DescriptionName.ToString());
    }
    
    return true;
}


bool UWarpContentSubSystem::WriteGameVersionToDataSource(const FGameVersion& InGameVersion)
{
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(AContentLog, !FolderDir.IsEmpty());
    const FString FileName = FGameVersion::Name.ToString() + TEXT(".json");
    const FString DescriptionJsonPath = FPaths::Combine(FolderDir, FileName);

    FString JsonString;
    const bool bOk = InGameVersion.VersionsToJson(JsonString);
    RETURN_ON_FAIL_BOOL_T(AContentLog, bOk, TEXT("Failed to JSON convert"));

    if (!FFileHelper::SaveStringToFile(JsonString, *DescriptionJsonPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
    {
        MG_ERROR(AContentLog, TEXT("Failed to write to file: %s"), *DescriptionJsonPath);
        return false;
    }

    return true;
}

bool UWarpContentSubSystem::WriteDescriptionToDataSourceFromJson(const FString& InDescriptionName,
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

bool UWarpContentSubSystem::WriteDescriptionToDataSource_Internal(const FName& DescriptionName, FBaseDescriptions& Description)
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
        MG_ERROR(AContentLog, TEXT("Failed to write to file: %s"), *DescriptionJsonPath);
        return false;
    }

    return true;
}

bool UWarpContentSubSystem::WriteDescriptionsToDataSource()
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

bool UWarpContentSubSystem::WriteDescriptionToDataSource(const FName& InDescriptionName)
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


bool UWarpContentSubSystem::SaveDescriptionToPlayFab(const FName& InDescriptionName)
{
    bool bSuccess = ReadDescriptionsFromDataSource();
    RETURN_ON_FAIL_BOOL(AContentLog, bSuccess);
    RETURN_ON_FAIL_BOOL(AContentLog, InDescriptionName.IsNone() || InDescriptionName.IsValid());
    RETURN_ON_FAIL_BOOL(AContentLog, RoleType_ == ERoleType::Developer);
    MG_COND_LOG(AContentLog, ContentFSM_ != nullptr, TEXT("ContentFSM_ already exist"));
    if (ContentFSM_ == nullptr) { ContentFSM_ = NewObject<UContentFSM>(this); }
    RETURN_ON_FAIL_BOOL(AContentLog, ContentFSM_);
    const FString FolderDir = GetGameDataSourceFilePath();
    RETURN_ON_FAIL_BOOL(AContentLog, !FolderDir.IsEmpty());

    TArray<FName> DescriptionNames;
    if (InDescriptionName.IsNone())
    {   
        RETURN_ON_FAIL_BOOL_T(AContentLog, GetDescriptionNames(DescriptionNames, FolderDir), TEXT("Failed to get description names"));    
    }
    else
    {
        DescriptionNames.Add(InDescriptionName);
    }

    TMap<FName, FString> DataToSaveJson;
    FGameVersion GameVersion;
    for (int i = 0; i < DescriptionNames.Num(); i++)
    {
        FString DescriptionJsonString;
        if (!GetFileJson(FolderDir, DescriptionNames[i].ToString() + TEXT(".json"), DescriptionJsonString))
        {
            MG_ERROR(AContentLog, TEXT("Could not load file %s to Json string"), *DescriptionNames[i].ToString());
            return false;
        }
        
        int32 Version = 0;
        FText FailReason;
        const bool bOk = TryGetVersionFromJson(DescriptionJsonString, Version, &FailReason);
        RETURN_ON_FAIL_BOOL_T(AContentLog, bOk, TEXT("Failed to convert Json to version: %s"), *FailReason.ToString());
        RETURN_ON_FAIL_BOOL_T(AContentLog, Version > 0, TEXT("Version is bad for %s"), *DescriptionNames[i].ToString());
        
        GameVersion = UpdateGameVersion(DescriptionNames[i], Version);
        DataToSaveJson.Add(DescriptionNames[i], DescriptionJsonString);
    }

    FString GameVersionJsonString;
    GameVersion.VersionsToJson(GameVersionJsonString);
    DataToSaveJson.Add(FGameVersion::Name, GameVersionJsonString);

    UWarpSwitchData SwitchData;
    SwitchData.DescriptionsToSaveJson = DataToSaveJson;
    ContentFSM_->Switch(NewObject<ULoginState>(ContentFSM_), &SwitchData);
    
    return true; 
}

FString UWarpContentSubSystem::GetGameDataSourceFilePath() const
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

FGameVersion UWarpContentSubSystem::UpdateGameVersion(FName InNewDescriptionName, int32 InNewDescriptionVersion)
{
    FGameVersion Versions = GetGameVersionFromDataSource();
    Versions.UpdateContentDescriptionVersion(InNewDescriptionName.ToString(), InNewDescriptionVersion);
    WriteGameVersionToDataSource(Versions);
    return Versions;
}

bool UWarpContentSubSystem::UpdateCachedGameData()
{
    ContentFSM_ = NewObject<UContentFSM>(this);
    RETURN_ON_FAIL_BOOL(AContentLog, ContentFSM_);
    UWarpSwitchData SwitchData;
    ContentFSM_->Switch(NewObject<ULoginState>(ContentFSM_), &SwitchData);
    return true;   
}


bool UWarpContentSubSystem::TryGetVersionFromJson(const FString& InJson, int32& OutVersion, FText* OutFailReason)
{
    OutVersion = 0;
    
    if (InJson.IsEmpty())
    {
        if (OutFailReason)
            *OutFailReason = FText::FromString(TEXT("JSON string is empty."));
        return false;
    }

    TSharedPtr<FJsonObject> RootObj;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InJson);

    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        if (OutFailReason)
            *OutFailReason = FText::FromString(TEXT("Failed to parse JSON into an object."));
        return false;
    }
    
    double VersionNumber = 0.0;
    if (RootObj->TryGetNumberField(TEXT("Version"), VersionNumber))
    {
        OutVersion = static_cast<int32>(VersionNumber);
        return true;
    }
    
    FString VersionString;
    if (RootObj->TryGetStringField(TEXT("Version"), VersionString))
    {
        OutVersion = FCString::Atoi(*VersionString);
        return true;
    }

    if (OutFailReason)
        *OutFailReason = FText::FromString(TEXT("Field 'Version' not found or not a number/string."));
    return false;
}

bool UWarpContentSubSystem::GetDescriptionNames(TArray<FName>& OutDescriptionNames,
    const FString& InFolderName) const
{
    OutDescriptionNames.Reset();
    RETURN_ON_FAIL_BOOL(AContentLog, !InFolderName.IsEmpty());

    TArray<FString> FoundFiles;
    const FString SearchPattern = FPaths::Combine(InFolderName, TEXT("*"));
    IFileManager::Get().FindFiles(FoundFiles, *SearchPattern, true, false);

    const FString GameVersionNameStr = FGameVersion::Name.ToString();

    OutDescriptionNames.Reserve(FoundFiles.Num());
    for (const FString& FileNameWithExt : FoundFiles)
    {
        const FString BaseName = FPaths::GetBaseFilename(FileNameWithExt);
        if (BaseName.Equals(GameVersionNameStr, ESearchCase::IgnoreCase))
        {
            continue;
        }

        OutDescriptionNames.Add(FName(*BaseName));
    }

    return true;
}

bool UWarpContentSubSystem::GetDescriptionNames(TArray<FName>& OutDescriptionNames,
    const TMap<FName, TUniquePtr<FBaseDescriptions>>& InDescriptionsMap) const
{
    OutDescriptionNames.Reset();
    OutDescriptionNames.Reserve(InDescriptionsMap.Num());

    for (const TPair<FName, TUniquePtr<FBaseDescriptions>>& Pair : InDescriptionsMap)
    {
        const FName KeyName = Pair.Key;
        OutDescriptionNames.Add(KeyName);
    }
    return true;
}

bool UWarpContentSubSystem::GetFileJson(const FString& InFolderName, const FString& InFileName, FString& OutJson) const
{
    const FString JsonPath = FPaths::Combine(InFolderName, InFileName);
    
    if (!IFileManager::Get().FileExists(*JsonPath))
    {
        MG_ERROR(AContentLog, TEXT("Json file does not exist: %s"), *JsonPath);
        return false;
    }
    
    if (!FFileHelper::LoadFileToString(OutJson, *JsonPath))
    {
        MG_ERROR(AContentLog, TEXT("Could not load file %s to Json string"), *JsonPath);
        return false;
    }

    return true;
}

ERoleType UWarpContentSubSystem::GetCurrentRoleType() const
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

void UWarpContentSubSystem::OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult)
{
    UE_LOG(LogTemp, Error, TEXT("PlayFab error: %s"),
           *ErrorResult.GenerateErrorReport());
}


void UWarpContentSubSystem::OnContentCheckedAndLoaded(bool InContentLoaded)
{
    MG_COND_WARNING(AContentLog, !InContentLoaded, TEXT("Failed to load content"));
    MG_LOG(AContentLog, TEXT("InContentLoaded: %d"), InContentLoaded);

    if (ContentFSM_)
        ContentFSM_ = nullptr;
    
    bContentLoaded_ = true;
    OnContentLoaded.Broadcast();
}


void UWarpContentSubSystem::OnSaveDone(bool InSaveSuccess)
{
	MG_LOG(AContentLog, TEXT("OnSaveDone: %d"), InSaveSuccess);

    if (ContentFSM_)
        ContentFSM_ = nullptr;
}
