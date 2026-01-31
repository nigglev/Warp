// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayFabStateManager.h"

#include "EPlayFabContentStates.h"
#include "MGLogs.h"
#include "Algo/ForEach.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayFabContentExtension.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/UnitDescription.h"

DEFINE_LOG_CATEGORY_STATIC(PFStateLog, Log, All);

void UPlayFabStateManager::SetState(const EPlayFabContentStates InNewState, FPlayFabStateManagerData& InData)
{
	if (CurrentState_ == InNewState)
	{
		return;
	}
	StateChangedLog(CurrentState_, InNewState);
	
	CurrentState_ = InNewState;
	OnStateSet(InData);
}

void UPlayFabStateManager::OnStateSet(FPlayFabStateManagerData& InData)
{
	if (CurrentState_ == EPlayFabContentStates::CheckUpdate)
	{
		HandleCheckUpdate(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::DownloadVersions)
	{
		HandleDownloadingVersions(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::CompareVersions)
	{
		HandleComparingVersions(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::GettingOutdatedContent)
	{
		HandleGettingOutdatedContent(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::UpdatingContent)
	{
		HandleUpdatingContent(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::UpdateDone)
	{
		HandleUpdateDone(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::UpdateFailed)
	{
		HandleUpdateFailed(InData);
	}
	else if (CurrentState_ == EPlayFabContentStates::Finished)
	{
		HandleFinished(InData);
	}
}

void UPlayFabStateManager::Start()
{
	MG_FUNC_LABEL(PFStateLog);
	ContextData_.Versions = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
	ContextData_.OldVersions = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
	ContextData_.CurrentDescriptionReader = MakeUnique<FDescriptionReaderBase>();
	SetState(EPlayFabContentStates::CheckUpdate, ContextData_);
}

void UPlayFabStateManager::OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess)
{
	if (ContextData_.Versions.Get() == InDescription)
	{
		if (InSuccess)
			SetState(EPlayFabContentStates::CompareVersions, ContextData_);
		else
			SetState(EPlayFabContentStates::UpdateFailed, ContextData_);
	}
	
	if (ContextData_.CurrentDescriptionReader.Get() == InDescription)
	{
		if (InSuccess)
		{
			if (ClientAPI_ != nullptr)
			{
				bool bOk = ContextData_.CurrentDescriptionReader->WriteGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>()));
				MG_COND_ERROR(PFStateLog, !bOk, TEXT("Failed to Write to GameplaySource"))	
			}
			
			if (ContextData_.DescriptionReaders.Num() > 0)
				SetState(EPlayFabContentStates::UpdatingContent, ContextData_);
			if (ContextData_.DescriptionReaders.Num() == 0)
				SetState(EPlayFabContentStates::UpdateDone, ContextData_);
		}
		else
		{
			SetState(EPlayFabContentStates::UpdateFailed, ContextData_);
		}
	}
}


void UPlayFabStateManager::HandleCheckUpdate(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	SetState(EPlayFabContentStates::DownloadVersions, InData);
}


void UPlayFabStateManager::HandleDownloadingVersions(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	if (ClientAPI_ == nullptr)
	{
		MG_ERROR(PFStateLog, TEXT("ClientAPI_ == nullptr"));
		SetState(EPlayFabContentStates::UpdateFailed, InData);
	}
	FAnyPlayFabPtr AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>(), ClientAPI_);
	AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>(), ClientAPI_);
	InData.Versions->ReadFromPlayFab(AnyApi, this);
}
	
void UPlayFabStateManager::HandleComparingVersions(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	if (ClientAPI_ == nullptr)
	{
		MG_ERROR(PFStateLog, TEXT("ClientAPI_ == nullptr"));
		SetState(EPlayFabContentStates::UpdateFailed, InData);
	}
	
	bool bSuccess = false;
	bSuccess = InData.OldVersions->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>()));
	
	if (!bSuccess)
	{
		SetState(EPlayFabContentStates::UpdateFailed, InData);
		MG_ERROR(PFStateLog, TEXT("Failed to read versions from source"));
		return;
	}
	if (InData.Versions->GetVersion() == InData.OldVersions->GetVersion())
	{
		SetState(EPlayFabContentStates::UpdateDone, InData);
		return;
	}
	
	if (InData.Versions->GetVersion() != InData.OldVersions->GetVersion())
	{
		MG_WARNING(PFStateLog, TEXT("Old version, please update"));
		SetState(EPlayFabContentStates::GettingOutdatedContent, InData);
	}
	
}

void UPlayFabStateManager::HandleGettingOutdatedContent(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	GetOutdatedDescriptions(InData.Versions->GetDescriptions(), InData.OldVersions->GetDescriptions(), InData.DescriptionReaders);
	if (InData.DescriptionReaders.Num() <= 0)
	{
		MG_WARNING(PFStateLog, TEXT("There was an update attempt, however there is no outdated content"));
		SetState(EPlayFabContentStates::UpdateDone, InData);
	}
	SetState(EPlayFabContentStates::UpdatingContent, InData);
}


void UPlayFabStateManager::HandleUpdatingContent(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	if (ClientAPI_ == nullptr)
	{
		MG_ERROR(PFStateLog, TEXT("ClientAPI_ == nullptr"));
		SetState(EPlayFabContentStates::UpdateFailed, InData);
	}
	
	if (InData.DescriptionReaders.Num() > 0)
	{
		InData.CurrentDescriptionReader = MoveTemp(InData.DescriptionReaders[InData.DescriptionReaders.Num() - 1]);
		InData.DescriptionReaders.RemoveAt(InData.DescriptionReaders.Num() - 1);	
	}

	FAnyPlayFabPtr AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>(), ClientAPI_);
	InData.CurrentDescriptionReader->ReadFromPlayFab(AnyApi, this);
}

void UPlayFabStateManager::HandleUpdateDone(FPlayFabStateManagerData& InData)
{
	MG_LOG(PFStateLog, TEXT("Versions Save Successful"));
	if (ClientAPI_ == nullptr)
	{
		MG_ERROR(PFStateLog, TEXT("ClientAPI_ == nullptr"));
		SetState(EPlayFabContentStates::UpdateFailed, InData);
	}
	bool bVersionSaveSuccessful = false;
	if (ClientAPI_ != nullptr)
	{
		bVersionSaveSuccessful = InData.Versions->WriteGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>()));
	}
	if (!bVersionSaveSuccessful)
	{
		MG_ERROR(PFStateLog, TEXT("Failed to Write Versions to GameplaySource"));
		SetState(EPlayFabContentStates::UpdateFailed, InData);
	}
		
	SetState(EPlayFabContentStates::Finished, InData);
}

void UPlayFabStateManager::HandleUpdateFailed(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	if (UWarpPlayfabContentSubSystem* SubSystem = Cast<UWarpPlayfabContentSubSystem>(GetOuter()))
	{
		SubSystem->OnContentCheckedAndLoaded(false);
	}
}

void UPlayFabStateManager::HandleFinished(FPlayFabStateManagerData& InData)
{
	MG_FUNC_LABEL(PFStateLog);
	if (UWarpPlayfabContentSubSystem* SubSystem = Cast<UWarpPlayfabContentSubSystem>(GetOuter()))
	{
		SubSystem->OnContentCheckedAndLoaded(true);
	}
	
}


void UPlayFabStateManager::GetOutdatedDescriptions(const FDescriptionVersions& LatestVersions,
                                                   const FDescriptionVersions& CurrentVersions, TArray<TUniquePtr<FDescriptionReaderBase>>& OutOutdated)
{
	OutOutdated.Reset();
	OutOutdated.Reserve(LatestVersions.Items.Num());

	Algo::ForEach(LatestVersions.Items, [&CurrentVersions, &OutOutdated](const FDescriptionVersion& Latest)
	{
		const FDescriptionVersion* Cur = Algo::FindByPredicate(CurrentVersions.Items,[&Latest](const FDescriptionVersion& X)
			{
				return X.DescriptionName == Latest.DescriptionName;
			});

		const int32 CurVer = Cur ? Cur->Version : 0;
		
		if (Latest.Version > CurVer)
		{
			const FName Key(*Latest.DescriptionName);
			if (TUniquePtr<FDescriptionReaderBase> Reader = WarpPlayfabContent::CreateReaderByKey(Key))
			{
				OutOutdated.Add(MoveTemp(Reader));
			}
		}
	});
}

void UPlayFabStateManager::StateChangedLog(EPlayFabContentStates InOldState, EPlayFabContentStates InNewState)
{
	const FString OldStateStr = StaticEnum<EPlayFabContentStates>()->GetNameStringByValue(static_cast<int64>(InOldState));
	const FString NewStateStr = StaticEnum<EPlayFabContentStates>()->GetNameStringByValue(static_cast<int64>(InNewState));
	MG_LOG(PFStateLog, TEXT("Match State Changed from %s to %s"), *OldStateStr, *NewStateStr);
}
