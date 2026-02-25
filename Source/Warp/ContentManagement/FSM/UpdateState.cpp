// Fill out your copyright notice in the Description page of Project Settings.


#include "UpdateState.h"

#include "JsonObjectConverter.h"
#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameVersion.h"
DEFINE_LOG_CATEGORY_STATIC(AUpdateState, Log, All);

void UUpdateState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	Super::OnEnter(InPrevState, InSwitchData);
	RETURN_ON_FAIL(AUpdateState, InSwitchData != nullptr);
	UWarpSwitchData* SwitchData = static_cast<UWarpSwitchData*>(InSwitchData);
	RETURN_ON_FAIL(AUpdateState, SwitchData->ClientAPI != nullptr);
	ClientAPI_ = SwitchData->ClientAPI;
	DownloadVersions();
}

bool UUpdateState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{
	return Super::OnExit(InNextState, InSwitchData);
}

bool UUpdateState::DownloadVersions()
{
	RETURN_ON_FAIL_BOOL(AUpdateState, ClientAPI_ != nullptr);
	PlayFab::ClientModels::FGetTitleDataRequest Request;
	Request.Keys.Add(TEXT("DescriptionVersions"));

	const bool bOk =ClientAPI_->GetTitleData(
	   Request,
	   PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate::CreateUObject(this, &UUpdateState::OnGetGameVersionTitleDataSuccess),
	   PlayFab::FPlayFabErrorDelegate::CreateUObject(this, &UUpdateState::OnGetGameVersionTitleDataError)
   );
	
	return bOk;
}

void UUpdateState::OnGetGameVersionTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result)
{
	const FString* Value = Result.Data.Find(TEXT("DescriptionVersions"));
	const bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct(*Value, &PlayFabVersion_);
	if (!Value)
	{
		MG_ERROR(AUpdateState, TEXT("GetTitleData: DescriptionVersions not found"));
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
		return;
	}
	if (!bParsed)
	{
		MG_ERROR(AUpdateState, TEXT("GetTitleData: failed to parse DescriptionVersions JSON: %s"), **Value);
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
		return;
	}

	FDescriptionVersions CurrentVersion = GetPlayfabContentSubsystem()->GetGameVersionFromDataSource();
	if (CurrentVersion.Version < 0)
	{
		MG_ERROR(AUpdateState, TEXT("Could not get current game version"));
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
		return;
	}
	if (CurrentVersion.Version == PlayFabVersion_.Version)
	{
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(true);
	}
	else
	{
		TArray<FString> OutdatedContent = GetContentToUpdate(CurrentVersion.Items, PlayFabVersion_.Items);
		bool bOk = UpdateContent(OutdatedContent);
		if (!bOk)
			GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
	}
}

void UUpdateState::OnGetGameVersionTitleDataError(const PlayFab::FPlayFabCppError& ErrorResult)
{
	MG_ERROR(AUpdateState, TEXT("GetTitleData failed: %s"), *ErrorResult.GenerateErrorReport());
	GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
}


TArray<FString> UUpdateState::GetContentToUpdate(const TArray<FDescriptionVersion>& InCurrentDescriptions,
                                                 const TArray<FDescriptionVersion>& InPlayFabDescriptions)
{
	TMap<FString, decltype(FDescriptionVersion::Version)> CurrentVersionByName;
	CurrentVersionByName.Reserve(InCurrentDescriptions.Num());

	for (const FDescriptionVersion& Cur : InCurrentDescriptions)
	{
		CurrentVersionByName.Add(Cur.DescriptionName, Cur.Version);
	}
	
	TSet<FString> OutdatedSet;
	OutdatedSet.Reserve(InPlayFabDescriptions.Num());

	for (const FDescriptionVersion& Remote : InPlayFabDescriptions)
	{
		const auto* LocalVersion = CurrentVersionByName.Find(Remote.DescriptionName);
		
		if (!LocalVersion || *LocalVersion != Remote.Version)
		{
			OutdatedSet.Add(Remote.DescriptionName);
		}
	}
	
	TArray<FString> OutdatedDescriptions = OutdatedSet.Array();
	return OutdatedDescriptions;
}

bool UUpdateState::UpdateContent(const TArray<FString>& InContentToUpdate)
{
	RETURN_ON_FAIL_BOOL(AUpdateState, !InContentToUpdate.IsEmpty());
	RETURN_ON_FAIL_BOOL(AUpdateState, ClientAPI_ != nullptr);

	PlayFab::ClientModels::FGetTitleDataRequest Request;
	for (int i = 0; i < InContentToUpdate.Num(); ++i)
	{
		Request.Keys.Add(InContentToUpdate[i]);
	}

	return ClientAPI_->GetTitleData(
		Request,
		PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate::CreateUObject(
			this, &UUpdateState::OnGetContentTitleDataSuccess),
		PlayFab::FPlayFabErrorDelegate::CreateUObject(
			this, &UUpdateState::OnGetContentTitleDataError)
	);
}

void UUpdateState::OnGetContentTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result)
{
	MG_FUNC_LABEL(AUpdateState);
	
	for (auto It = Result.Data.CreateConstIterator(); It; ++It)
	{
		const FString& Key = It.Key();
		const FString& Value = It.Value();
		GetPlayfabContentSubsystem()->WriteDescriptionToDataSourceFromJson(Key, Value);
	}
	GetPlayfabContentSubsystem()->WriteGameVersionToDataSource(PlayFabVersion_);
	GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(true);
}

void UUpdateState::OnGetContentTitleDataError(const PlayFab::FPlayFabCppError& ErrorResult)
{
	MG_ERROR(AUpdateState, TEXT("GetTitleData failed: %s"), *ErrorResult.GenerateErrorReport());
	GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
}
