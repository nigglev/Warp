// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayFabStateManager.h"

#include "EPlayFabContentStates.h"
#include "MGLogs.h"
#include "Algo/ForEach.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayFabContentExtension.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/UnitDescription.h"

DEFINE_LOG_CATEGORY_STATIC(PFStateLog, Log, All);

using FReaderFactory = TFunction<TUniquePtr<FDescriptionReaderBase>()>;

static TMap<FName, FReaderFactory> GReaderFactories = {
	{ TEXT("UnitDescriptions"), [](){ return MakeUnique<FUStructDescriptionReader<FUnitDescriptions>>(); } },
};

TUniquePtr<FDescriptionReaderBase> CreateReaderByKey(const FName& Key)
{
	if (const FReaderFactory* Factory = GReaderFactories.Find(Key))
	{
		return (*Factory)();
	}
	return nullptr;
}

void UPlayFabStateManager::SetOwner(UWarpPlayfabContentSubSystem* InOwner)
{
	if (InOwner != nullptr)
	{
		MG_FUNC_LABEL(PFStateLog);
		Owner_ = InOwner;
	}
	else
	{
		MG_ERROR(PFStateLog, TEXT("Invalid Owner"));
	}
		
}

void UPlayFabStateManager::SetState(const EPlayFabContentStates InNewState, const FPlayFabStateManagerData* InData)
{
	if (CurrentState_ == InNewState)
	{
		return;
	}
	StateChangedLog(CurrentState_, InNewState);
	
	CurrentState_ = InNewState;

	OnStateSet(InData);
}

void UPlayFabStateManager::OnStateSet(const FPlayFabStateManagerData* InData)
{
	if (CurrentState_ == EPlayFabContentStates::StartLogin)
	{
		HandleStartLogin();
	}
	else if (CurrentState_ == EPlayFabContentStates::ProcessingLogin)
	{
		HandleProcessingLogin();
	}
	else if (CurrentState_ == EPlayFabContentStates::LoginFailure)
	{
		HandleLoginFailure();
	}
	else if (CurrentState_ == EPlayFabContentStates::LoginSuccess)
	{
		HandleLoginSuccess();
	}
	else if (CurrentState_ == EPlayFabContentStates::SaveDescriptions)
	{
		const FString DescName = InData ? InData->DescriptionName : FString();
		HandleSavingDescriptions(DescName);
	}
	else if (CurrentState_ == EPlayFabContentStates::DownloadVersions)
	{
		HandleDownloadingVersions();
	}
	else if (CurrentState_ == EPlayFabContentStates::CompareVersions)
	{
		HandleComparingVersions();
	}
	else if (CurrentState_ == EPlayFabContentStates::UpdatingContent)
	{
		HandleUpdatingContent();
	}
	else if (CurrentState_ == EPlayFabContentStates::GettingOutdatedContent)
	{
		HandleGettingOutdatedContent();
	}
	else if (CurrentState_ == EPlayFabContentStates::UpdateDone)
	{
		HandleUpdateDone();
	}
	else if (CurrentState_ == EPlayFabContentStates::Failure)
	{
		HandleFailure();
	}
	else if (CurrentState_ == EPlayFabContentStates::Finished)
	{
		HandleFinished();
	}
}


void UPlayFabStateManager::OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess)
{
	if (Versions_.Get() == InDescription)
	{
		if (InSuccess)
			SetState(EPlayFabContentStates::CompareVersions);
		else
			SetState(EPlayFabContentStates::Failure);
	}

	if (CurrentDescriptionReader_.Get() == InDescription)
	{
		if (InSuccess)
		{
			if (ServerAPI_ != nullptr)
			{
				bool bOk = CurrentDescriptionReader_->WriteGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
				MG_COND_ERROR(PFStateLog, !bOk, TEXT("Failed to Write to GameplaySource"))	
			}
			if (ClientAPI_ != nullptr)
			{
				bool bOk = CurrentDescriptionReader_->WriteGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>()));
				MG_COND_ERROR(PFStateLog, !bOk, TEXT("Failed to Write to GameplaySource"))	
			}
			
			if (OutDatedDescriptionReaders_.Num() > 0)
				SetState(EPlayFabContentStates::UpdatingContent);
			if (OutDatedDescriptionReaders_.Num() == 0)
				SetState(EPlayFabContentStates::UpdateDone);
		}
		else
		{
			SetState(EPlayFabContentStates::Failure);
		}
	}
}


void UPlayFabStateManager::OnDescriptionSavingResult(FDescriptionReaderBase* InDescription, bool InSuccess)
{
	if (ServerAPI_ != nullptr)
	{
		if (CurrentDescriptionReader_.Get() == InDescription && InSuccess)
		{
			MG_LOG(PFStateLog, TEXT("Description Save Successful"));
			bDescriptionSaveSuccessful_ = true;

			if (Versions_ == nullptr)
			{
				MG_ERROR(PFStateLog, TEXT("Versions_ invalid"))
				SetState(EPlayFabContentStates::Failure);
			}
			
			FDescriptionVersions& V = Versions_->GetDescriptions();
			V.UpdateVersions(CurrentDescriptionReader_->GetName(), CurrentDescriptionReader_->GetVersion());
			Versions_->SaveToPlayFab(ServerAPI_, this);
		}

		if (Versions_.Get() == InDescription && InSuccess && bDescriptionSaveSuccessful_)
		{
			SetState(EPlayFabContentStates::UpdateDone);
		}	
	}
}

void UPlayFabStateManager::HandleStartLogin()
{
	MG_FUNC_LABEL(PFStateLog);
	LoginToPlayFab();
	SetState(EPlayFabContentStates::ProcessingLogin);
}


void UPlayFabStateManager::OnLoginResult(const bool InLoginRes)
{
	MG_FUNC_LABEL(PFStateLog);
	RETURN_ON_FAIL_T(PFStateLog, LoginInfo_, TEXT("Failed to create LoginInfo"));
	
	if (InLoginRes)
		SetState(EPlayFabContentStates::LoginSuccess);
	else
	{
		SetState(EPlayFabContentStates::LoginFailure);
	}
}

void UPlayFabStateManager::HandleProcessingLogin()
{
	MG_FUNC_LABEL(PFStateLog);
}

void UPlayFabStateManager::HandleLoginFailure()
{
	MG_FUNC_LABEL(PFStateLog);
}

void UPlayFabStateManager::HandleLoginSuccess()
{
	MG_FUNC_LABEL(PFStateLog);
	SetState(EPlayFabContentStates::DownloadVersions);
}

void UPlayFabStateManager::HandleSavingDescriptions(const FString& InDescriptionName)
{
	if (!Versions_)
	{
		Versions_ = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
	}

	if (!CurrentDescriptionReader_)
	{
		CurrentDescriptionReader_ = CreateReaderByKey(FName(*InDescriptionName));
	}
	
	
	bool bOk = CurrentDescriptionReader_->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
	RETURN_ON_FAIL_T(PFStateLog, bOk, TEXT("Failed to DescriptionReader from source"));
	bOk = Versions_->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
	RETURN_ON_FAIL_T(PFStateLog, bOk, TEXT("Failed to VersionsReader from source"));
	
	CurrentDescriptionReader_->SaveToPlayFab(ServerAPI_, this);
}

void UPlayFabStateManager::HandleDownloadingVersions()
{
	MG_FUNC_LABEL(PFStateLog);
	if (!Versions_)
	{
		Versions_ = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
	}

	MG_COND_ERROR_SHORT(PFStateLog, ServerAPI_ == nullptr && ClientAPI_ == nullptr);
	
	FAnyPlayFabPtr AnyApi;
	if (ServerAPI_ != nullptr)
	{
		AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>(), ServerAPI_);
	}
	else if (ClientAPI_ != nullptr)
	{
		AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>(), ClientAPI_);
	}
	Versions_->ReadFromPlayFab(AnyApi, this);
}

void UPlayFabStateManager::HandleComparingVersions()
{
	MG_FUNC_LABEL(PFStateLog);
	if (!CurrentVersions_)
	{
		CurrentVersions_ = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
	}
	
	bool bSuccess = false;
	if (ServerAPI_ != nullptr)
	{
		bSuccess = CurrentVersions_->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
	}
	if (ClientAPI_ != nullptr)
	{
		bSuccess = CurrentVersions_->ReadGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>()));
	}
	
	if (!bSuccess)
	{
		SetState(EPlayFabContentStates::Failure);
		MG_ERROR(PFStateLog, TEXT("Failed to read versions from source"));
		return;
	}
	if (Versions_->GetVersion() == CurrentVersions_->GetVersion())
	{
		SetState(EPlayFabContentStates::Finished);
		return;
	}
	
	if (Versions_->GetVersion() != CurrentVersions_->GetVersion())
	{
		MG_WARNING(PFStateLog, TEXT("Old version, please update"));
		SetState(EPlayFabContentStates::GettingOutdatedContent);
	}
	
}

void UPlayFabStateManager::HandleUpdatingContent()
{	
	if (OutDatedDescriptionReaders_.Num() > 0)
	{
		CurrentDescriptionReader_ = MoveTemp(OutDatedDescriptionReaders_[OutDatedDescriptionReaders_.Num() - 1]);
		OutDatedDescriptionReaders_.RemoveAt(OutDatedDescriptionReaders_.Num() - 1);	
	}
	
	FAnyPlayFabPtr AnyApi;
	if (ServerAPI_ != nullptr)
	{
		AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>(), ServerAPI_);
	}
	if (ClientAPI_ != nullptr)
	{
		AnyApi = FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>(), ClientAPI_);
	}
	CurrentDescriptionReader_->ReadFromPlayFab(AnyApi, this);
}

void UPlayFabStateManager::HandleGettingOutdatedContent()
{
	MG_FUNC_LABEL(PFStateLog);
	GetOutdatedDescriptions(Versions_->GetDescriptions(), CurrentVersions_->GetDescriptions(), OutDatedDescriptionReaders_);
	if (OutDatedDescriptionReaders_.Num() <= 0)
	{
		MG_WARNING(PFStateLog, TEXT("There was an update attempt, however there is no outdated content"));
		SetState(EPlayFabContentStates::UpdateDone);
	}
	SetState(EPlayFabContentStates::UpdatingContent);
}

void UPlayFabStateManager::HandleUpdateDone()
{
	MG_LOG(PFStateLog, TEXT("Versions Save Successful"));
	if (ServerAPI_ != nullptr)
	{
		bVersionSaveSuccessful_ = Versions_->WriteGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabServerPtr>()));
	}
	if (ClientAPI_ != nullptr)
	{
		bVersionSaveSuccessful_ = Versions_->WriteGameplaySource(FAnyPlayFabPtr(TInPlaceType<PlayFabClientPtr>()));
	}
	if (!bVersionSaveSuccessful_)
	{
		MG_ERROR(PFStateLog, TEXT("Failed to Write Versions to GameplaySource"));
		SetState(EPlayFabContentStates::Failure);
	}
		
	SetState(EPlayFabContentStates::Finished);
}

void UPlayFabStateManager::HandleFailure()
{
	Reset();
}

void UPlayFabStateManager::HandleFinished()
{
	Reset();
	Owner_->BroadcastContentIsLoaded(true);
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

			if (TUniquePtr<FDescriptionReaderBase> Reader = CreateReaderByKey(Key))
			{
				OutOutdated.Add(MoveTemp(Reader));
			}
		}
	});
}

bool UPlayFabStateManager::LoginToPlayFab()
{
	MG_COND_ERROR(PFStateLog, LoginInfo_ != nullptr, TEXT("LoginInfo_ already exist"));
	LoginInfo_ = NewObject<UPlayFabLoginInfo>();
	LoginInfo_->OnLoginResult.AddUObject(this, &UPlayFabStateManager::OnLoginResult);
	ENetMode NetMode = GetWorld()->GetNetMode();
    bool bSuccess;
	TOptional<FString> SecretKey = WarpPlayfabContent::ReadSecret();
	if (SecretKey.IsSet() && NetMode != NM_Client)
	{
		UPlayFabRuntimeSettings* Settings = GetMutableDefault<UPlayFabRuntimeSettings>();
		RETURN_ON_FAIL_BOOL(PFStateLog, Settings != nullptr);
		Settings->DeveloperSecretKey = SecretKey.GetValue();
		MG_LOG(PFStateLog, TEXT("PlayFab secret set"));
		ServerAPI_ = IPlayFabModuleInterface::Get().GetServerAPI();
		MG_COND_ERROR(PFStateLog, ServerAPI_ == nullptr, TEXT("Server API missing"));
		bSuccess = WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FServerTag>(ServerAPI_, LoginInfo_, TEXT("DedicatedServer"));
	}
	else
	{
		ClientAPI_ = IPlayFabModuleInterface::Get().GetClientAPI();
		MG_COND_ERROR(PFStateLog, ClientAPI_ == nullptr, TEXT("Client API missing"));
		bSuccess = WarpPlayfabContent::LoginWithCustomId<WarpPlayfabContent::FClientTag>(ClientAPI_, LoginInfo_, TEXT("DevClient"));
	}

	return bSuccess;
}


void UPlayFabStateManager::Reset()
{
	Versions_ = nullptr;
	CurrentVersions_ = nullptr;
	
	DescriptionReaders_.Empty();
	OutDatedDescriptionReaders_.Empty();
	CurrentDescriptionReader_ = nullptr;

	bDescriptionSaveSuccessful_ = false;
	bVersionSaveSuccessful_ = false;
}

void UPlayFabStateManager::StateChangedLog(EPlayFabContentStates InOldState, EPlayFabContentStates InNewState)
{
	const FString OldStateStr = StaticEnum<EPlayFabContentStates>()->GetNameStringByValue(static_cast<int64>(InOldState));
	const FString NewStateStr = StaticEnum<EPlayFabContentStates>()->GetNameStringByValue(static_cast<int64>(InNewState));
	MG_LOG(PFStateLog, TEXT("Match State Changed from %s to %s"), *OldStateStr, *NewStateStr);
}
