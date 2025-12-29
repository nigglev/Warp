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
    StateManager_ = CreateDefaultSubobject<UPlayFabStateManager>(TEXT("PFStateManager"));
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
    StateManager_->SetOwner(this);
    StateManager_->SetState(EPlayFabContentStates::Login);
}

void UWarpPlayfabContentSubSystem::SaveDescriptionToPlayFab(const FString& InDescriptionName)
{
    if (!IsClient())
    {
        if (StateManager_->GetState() == EPlayFabContentStates::UpdatePending)
        {
            MG_ERROR(ContentLog, TEXT("Please update to latest version before saving to PlayFab"));
            return;
        }
        FPlayFabStateManagerData StateData;
        StateData.DescriptionName = InDescriptionName;
        StateManager_->SetState(EPlayFabContentStates::SaveDescriptions, &StateData);
    }
}

void UWarpPlayfabContentSubSystem::UpdateContent()
{
#if WITH_EDITOR

    if (StateManager_->GetState() != EPlayFabContentStates::UpdatePending)
    {
        MG_WARNING(ContentLog, TEXT("Nothing to update; You have latest version"));
    }
    StateManager_->SetState(EPlayFabContentStates::UpdatingContent);
    
#else
#endif
}


void UWarpPlayfabContentSubSystem::DownloadUnits()
{
    // using namespace PlayFab::ClientModels;
    // RETURN_ON_FAIL(ContentLog, ClientAPI_.IsValid())
    //
    // FGetTitleDataRequest Request;
    // Request.Keys.Add(TEXT("Units"));

//    ServerAPI_->GetTitleData()
    // ClientAPI_->GetTitleData(
    //     Request,
    //     PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate::CreateUObject(
    //         this, &UWarpPlayfabContentSubSystem::OnGetTitleDataSuccess),
    //     PlayFab::FPlayFabErrorDelegate::CreateUObject(
    //         this, &UWarpPlayfabContentSubSystem::OnPlayFabError));
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

bool UWarpPlayfabContentSubSystem::IsClient() const
{
    ENetMode NetMode = GetWorld()->GetNetMode();
    return  (NetMode != NM_Client);
}
