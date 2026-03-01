// Fill out your copyright notice in the Description page of Project Settings.


#include "SaveState.h"

#include "BaseState.h"
#include "MGLogs.h"
#include "PlayFabServerDataModels.h"
#include "Core/PlayFabServerAPI.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
DEFINE_LOG_CATEGORY_STATIC(ASaveState, Log, All);

namespace
{
	struct FTitleDataBatchState
	{
		PlayFabServerPtr Api;
		TArray<TPair<FName, FString>> Items;
		int32 Index = 0;
		int32 Succeeded = 0;
		int32 Failed = 0;

		TWeakObjectPtr<UWarpPlayfabContentSubSystem> Subsystem;
	};

	void SendNext_TitleDataBatch(const TSharedRef<FTitleDataBatchState>& State)
	{
		if (!State->Api || !State->Subsystem.IsValid())
		{
			MG_ERROR(ASaveState, TEXT("Api or Subsystem is invalid"));
			return;
		}

		if (State->Index >= State->Items.Num())
		{
			const bool bAllOk = State->Failed == 0;
			MG_LOG(ASaveState, TEXT("TitleData batch finished. Ok=%d Fail=%d"), State->Succeeded, State->Failed);
			State->Subsystem->OnSaveDone(bAllOk);
			return;
		}

		const TPair<FName, FString> Pair = State->Items[State->Index++];
		const FString KeyStr = Pair.Key.ToString();
		const FString& JsonStr = Pair.Value;
		
		if (Pair.Key.IsNone() || JsonStr.IsEmpty())
		{
			++State->Failed;
			MG_ERROR(ASaveState, TEXT("Skipping invalid TitleData item. Key='%s' JsonEmpty=%d"),
				*KeyStr, JsonStr.IsEmpty() ? 1 : 0);
			SendNext_TitleDataBatch(State);
			return;
		}

		PlayFab::ServerModels::FSetTitleDataRequest Request;
		Request.Key = KeyStr;
		Request.Value = JsonStr;

		PlayFab::UPlayFabServerAPI::FSetTitleDataDelegate OnOk;
		OnOk.BindWeakLambda(State->Subsystem.Get(), [State, KeyStr](const PlayFab::ServerModels::FSetTitleDataResult&)
		{
			++State->Succeeded;
			MG_LOG(ASaveState, TEXT("SetTitleData OK: %s"), *KeyStr);
			SendNext_TitleDataBatch(State);
		});

		PlayFab::FPlayFabErrorDelegate OnErr;
		OnErr.BindWeakLambda(State->Subsystem.Get(), [State, KeyStr](const PlayFab::FPlayFabCppError& Err)
		{
			++State->Failed;
			MG_ERROR(ASaveState, TEXT("SetTitleData FAILED: %s | %s"),
				*KeyStr, *Err.GenerateErrorReport());
			SendNext_TitleDataBatch(State);
		});

		const bool bStarted = State->Api->SetTitleData(Request, OnOk, OnErr);
		if (!bStarted)
		{
			++State->Failed;
			MG_ERROR(ASaveState, TEXT("Api->SetTitleData could not start for key: %s"), *KeyStr);
			SendNext_TitleDataBatch(State);
		}
	}
}

void USaveState::OnEnter(UContentFSMState* InPrevState, UContentFSMSwitchData* InSwitchData)
{
	Super::OnEnter(InPrevState, InSwitchData);
	RETURN_ON_FAIL(ASaveState, InSwitchData != nullptr);
	UWarpSwitchData* SwitchData = static_cast<UWarpSwitchData*>(InSwitchData);
	RETURN_ON_FAIL(ASaveState, SwitchData != nullptr);
	RETURN_ON_FAIL(ASaveState, SwitchData->ServerAPI != nullptr);
	RETURN_ON_FAIL(ASaveState, SwitchData->DescriptionsToSaveJson.Num() > 0);
	SaveDescriptionsBatchToPlayFab(SwitchData->DescriptionsToSaveJson, SwitchData->ServerAPI);
}

bool USaveState::OnExit(UContentFSMState* InNextState, UContentFSMSwitchData* InSwitchData)
{
	return Super::OnExit(InNextState, InSwitchData);
}

bool USaveState::SaveDescriptionsBatchToPlayFab(const TMap<FName, FString>& InDescriptionsToSaveJson,
	const PlayFabServerPtr& InPlayFabAPI)
{
	UWarpPlayfabContentSubSystem* Subsystem = GetPlayfabContentSubsystem();
	RETURN_ON_FAIL_BOOL(ASaveState, Subsystem != nullptr);

	if (!InPlayFabAPI)
	{
		MG_ERROR(ASaveState, TEXT("InPlayFabAPI is invalid"));
		Subsystem->OnSaveDone(false);
		return false;
	}

	if (InDescriptionsToSaveJson.Num() == 0)
	{
		MG_LOG(ASaveState, TEXT("No descriptions to save"));
		Subsystem->OnSaveDone(true);
		return true;
	}

	TSharedRef<FTitleDataBatchState> State = MakeShared<FTitleDataBatchState>();
	State->Api = InPlayFabAPI;
	State->Subsystem = Subsystem;
	State->Items.Reserve(InDescriptionsToSaveJson.Num());

	for (const TPair<FName, FString>& It : InDescriptionsToSaveJson)
	{
		State->Items.Add(It);
	}

	SendNext_TitleDataBatch(State);
	return true;
}
