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
	DownloadVersions(InSwitchData);
}

bool UUpdateState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{
	return Super::OnExit(InNextState, InSwitchData);
}

bool UUpdateState::DownloadVersions(UContentFSMSwitchData* InSwitchData)
{
	RETURN_ON_FAIL_BOOL(AUpdateState, InSwitchData != nullptr);
	UWarpSwitchData* SwitchData = static_cast<UWarpSwitchData*>(InSwitchData);
	RETURN_ON_FAIL_BOOL(AUpdateState, SwitchData->ClientAPI != nullptr);
	

	PlayFab::ClientModels::FGetTitleDataRequest Request;
	Request.Keys.Add(TEXT("DescriptionVersions"));

	const bool bOk = SwitchData->ClientAPI->GetTitleData(
	   Request,
	   PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate::CreateUObject(this, &UUpdateState::OnGetTitleDataSuccess),
	   PlayFab::FPlayFabErrorDelegate::CreateUObject(this, &UUpdateState::OnGetTitleDataError)
   );
	
	return bOk;
}

void UUpdateState::OnGetTitleDataSuccess(const PlayFab::ClientModels::FGetTitleDataResult& Result)
{
	const FString* Value = Result.Data.Find(TEXT("DescriptionVersions"));
	FDescriptionVersions Versions;
	const bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct(*Value, &Versions);
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
	if (CurrentVersion.Version <= 0)
	{
		MG_ERROR(AUpdateState, TEXT("Could not get current game version"));
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
		return;
	}
	if (CurrentVersion.Version == Versions.Version)
	{
		GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(true);
	}
	else
	{
		GetNewContentFromPlayFab(CurrentVersion.Items);
	}
}

void UUpdateState::OnGetTitleDataError(const PlayFab::FPlayFabCppError& ErrorResult)
{
	MG_ERROR(AUpdateState, TEXT("GetTitleData failed: %s"), *ErrorResult.GenerateErrorReport());
	GetPlayfabContentSubsystem()->OnContentCheckedAndLoaded(false);
}


void UUpdateState::GetNewContentFromPlayFab(const TArray<FDescriptionVersion>& InDescriptions)
{
	
}


