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

void UPlayFabStateManager::OnDescriptionReadingResult(FDescriptionReaderBase* InDescription, bool InSuccess)
{
	if (Versions_.Get() == InDescription)
	{
		if (InSuccess)
			SetState(EPlayFabContentStates::CompareVersions);
		else
		{
			SetState(EPlayFabContentStates::Failure);
		}
	}

	if (CurrentDescriptionReader_.Get() == InDescription && InSuccess)
	{
		#if WITH_EDITOR
		
			bool bOk = CurrentDescriptionReader_->WriteGameplaySource();
			MG_COND_ERROR(PFStateLog, !bOk, TEXT("Failed to Write to GameplaySource"))
		
		#else
		#endif

		if (OutDatedDescriptionReaders_.Num() > 0)
			SetState(EPlayFabContentStates::UpdatingContent);
		if (OutDatedDescriptionReaders_.Num() == 0)
			SetState(EPlayFabContentStates::UpdateDone);
	}
}


void UPlayFabStateManager::OnDescriptionSavingResult(FDescriptionReaderBase* InDescription, bool InSuccess)
{
#if WITH_EDITOR
	if (CurrentDescriptionReader_.Get() == InDescription && InSuccess)
	{
		MG_LOG(PFStateLog, TEXT("Description Save Successful"));
		bDescriptionSaveSuccessful_ = true;
		
		FDescriptionVersions& V = Versions_->GetDescriptions();
		V.UpdateVersions(CurrentDescriptionReader_->GetName(), CurrentDescriptionReader_->GetVersion());
		Versions_->SaveToPlayFab(ServerAPI_, this);
	}

	if (Versions_.Get() == InDescription && InSuccess && bDescriptionSaveSuccessful_)
	{
		SetState(EPlayFabContentStates::UpdateDone);
	}
		
#else
#endif
}


void UPlayFabStateManager::OnStateSet(const FPlayFabStateManagerData* InData)
{
	if (CurrentState_ == EPlayFabContentStates::Login)
	{
		HandleLogin();
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
	else if (CurrentState_ == EPlayFabContentStates::UpdatePending)
	{
		HandleUpdatePending();
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

void UPlayFabStateManager::HandleLogin()
{
	MG_FUNC_LABEL(PFStateLog);
	
	if (LoginToPlayFab())
		SetState(EPlayFabContentStates::LoginSuccess);
	else
		SetState(EPlayFabContentStates::LoginFailure);
}

void UPlayFabStateManager::HandleLoginFailure()
{
	MG_FUNC_LABEL(PFStateLog);
	SetState(EPlayFabContentStates::Login);
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
	
	
	bool bOk = CurrentDescriptionReader_->ReadGameplaySource();
	RETURN_ON_FAIL_T(PFStateLog, bOk, TEXT("Failed to DescriptionReader from source"));
	bOk = Versions_->ReadGameplaySource();
	RETURN_ON_FAIL_T(PFStateLog, bOk, TEXT("Failed to VersionsReader from source"));
	
	CurrentDescriptionReader_->SaveToPlayFab(ServerAPI_, this);
}

void UPlayFabStateManager::HandleDownloadingVersions()
{
	MG_FUNC_LABEL(PFStateLog);
	Versions_ = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
	if (ServerAPI_ != nullptr)
	{
		Versions_->ReadFromPlayFab(ServerAPI_, this);
	}
}

void UPlayFabStateManager::HandleComparingVersions()
{
	MG_FUNC_LABEL(PFStateLog);
	if (ServerAPI_ != nullptr)
	{
		CurrentVersions_ = MakeUnique<FUStructDescriptionReader<FDescriptionVersions>>();
		bool bSuccess = CurrentVersions_->ReadGameplaySource();
		if (!bSuccess)
		{
			SetState(EPlayFabContentStates::Failure);
			MG_ERROR(PFStateLog, TEXT("Failed to read versions from source"));
			return;
		}
		if (Versions_->GetVersion() != CurrentVersions_->GetVersion())
		{
			MG_WARNING(PFStateLog, TEXT("Old version, please update"));
			SetState(EPlayFabContentStates::UpdatePending);
			return;
		}
		if (Versions_->GetVersion() == CurrentVersions_->GetVersion())
		{
			SetState(EPlayFabContentStates::Finished);
		}
	}
	

	
	// if (Versions_->GetVersion() == CurrentVersions.GetVersion())
	// 	SetState(EPlayFabContentStates::UpdateDone);
	// 	
	// 		
	// GetOutdatedDescriptions(Versions_->GetDescriptions(), CurrentVersions.GetDescriptions(), OutDatedDescriptionReaders_);
	//
	// if (OutDatedDescriptionReaders_.Num() == 0)
	// 	SetState(EPlayFabContentStates::UpdateDone);
	//
	// SetState(EPlayFabContentStates::UpdatingContent);
}

void UPlayFabStateManager::HandleUpdatingContent()
{	
	if (OutDatedDescriptionReaders_.Num() > 0)
	{
		CurrentDescriptionReader_ = MoveTemp(OutDatedDescriptionReaders_[OutDatedDescriptionReaders_.Num() - 1]);
		OutDatedDescriptionReaders_.RemoveAt(OutDatedDescriptionReaders_.Num() - 1);	
	}
	CurrentDescriptionReader_->ReadFromPlayFab(ServerAPI_, this);
}

void UPlayFabStateManager::HandleUpdatePending()
{
	GetOutdatedDescriptions(Versions_->GetDescriptions(), CurrentVersions_->GetDescriptions(), OutDatedDescriptionReaders_);
}

void UPlayFabStateManager::HandleUpdateDone()
{
	MG_LOG(PFStateLog, TEXT("Versions Save Successful"));
	bVersionSaveSuccessful_ = Versions_->WriteGameplaySource();
	SetState(EPlayFabContentStates::Finished);
}

void UPlayFabStateManager::HandleFailure()
{
	Versions_ = nullptr;
	CurrentVersions_ = nullptr;
	
	DescriptionReaders_.Empty();
	OutDatedDescriptionReaders_.Empty();
	CurrentDescriptionReader_ = nullptr;

	bDescriptionSaveSuccessful_ = false;
	bVersionSaveSuccessful_ = false;
}

void UPlayFabStateManager::HandleFinished()
{
	Versions_ = nullptr;
	CurrentVersions_ = nullptr;
	
	DescriptionReaders_.Empty();
	OutDatedDescriptionReaders_.Empty();
	CurrentDescriptionReader_ = nullptr;

	bDescriptionSaveSuccessful_ = false;
	bVersionSaveSuccessful_ = false;
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
	LoginInfo_ = NewObject<UPlayFabLoginInfo>();
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

void UPlayFabStateManager::StateChangedLog(EPlayFabContentStates InOldState, EPlayFabContentStates InNewState)
{
	const FString OldStateStr = StaticEnum<EPlayFabContentStates>()->GetNameStringByValue(static_cast<int64>(InOldState));
	const FString NewStateStr = StaticEnum<EPlayFabContentStates>()->GetNameStringByValue(static_cast<int64>(InNewState));
	MG_LOG(PFStateLog, TEXT("Match State Changed from %s to %s"), *OldStateStr, *NewStateStr);
}
