// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayfabContentSubSystem.h"

#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"

#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"

DEFINE_LOG_CATEGORY_STATIC(ContentLog, Log, All);

void UWarpPlayfabContentSubSystem::Initialize(FSubsystemCollectionBase& InCollection)
{
    Super::Initialize(InCollection);

    ClientAPI_ = IPlayFabModuleInterface::Get().GetClientAPI();
    MG_COND_ERROR(ContentLog, ClientAPI_ == nullptr, TEXT("Client API missing"));
}

void UWarpPlayfabContentSubSystem::DownloadUnits()
{
    using namespace PlayFab::ClientModels;
    RETURN_ON_FAIL(ContentLog, ClientAPI_.IsValid())
    
    FGetTitleDataRequest Request;
    Request.Keys.Add(TEXT("Units"));

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
