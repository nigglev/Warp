// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DescriptionReaderBase.h"
#include "UObject/Object.h"
#include "JsonObjectConverter.h"
#include "MGLogs.h"
#include "Core/PlayFabServerAPI.h"
#include "Warp/ContentManagement/StaticDescriptions/UnitDescription.h"

DEFINE_LOG_CATEGORY_STATIC(DescriptionReaderLog, Log, All);

template<typename TUStruct>
class FUStructDescriptionReader: public FDescriptionReaderBase
{
public:

	virtual FString GetName() const override
	{
		FString StructName = TUStruct::StaticStruct()->GetName();
		return StructName;
	}
	
	virtual int32 GetVersion() const override
	{
		return Descriptions_.Version;
	}

	virtual bool ReadGameplaySource() override
	{
		FString RelativeContentDirectory = FPaths::ProjectContentDir();
		FString FileName = FString::Printf(TEXT("%s.json"), *GetName());
		FString Filepath = FPaths::Combine(RelativeContentDirectory, TEXT("../GameDataSource"), FileName);
		
		FString JsonString;
		if (!FFileHelper::LoadFileToString(JsonString, *Filepath))
		{
			if (Descriptions_.Items.IsEmpty())
			{
				Descriptions_.Items.Emplace();
			}
			
			bool bOk = FJsonObjectConverter::UStructToJsonObjectString(Descriptions_, JsonString);
			RETURN_ON_FAIL_BOOL_T(DescriptionReaderLog, bOk, TEXT("Failed to JSON convert"));
			
			bOk = FFileHelper::SaveStringToFile(JsonString, *Filepath);
			MG_COND_ERROR(DescriptionReaderLog, !bOk, TEXT("Failed to save file: %s"), *Filepath);
			MG_COND_LOG(DescriptionReaderLog, bOk, TEXT("Test description has been saves to file: %s"), *Filepath);
		}
		
		const bool bOk = FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &Descriptions_);
		MG_COND_ERROR(DescriptionReaderLog, !bOk, TEXT("Failed to JSON convert"));
		return bOk;
	}
	
	virtual bool SaveToPlayFab(const PlayFabServerPtr& InPlayFabAPI, UWarpPlayfabContentSubSystem* InUserObject) override
	{
		RETURN_ON_FAIL_BOOL(DescriptionReaderLog, InPlayFabAPI != nullptr);
		RETURN_ON_FAIL_BOOL(DescriptionReaderLog, InUserObject != nullptr);
		RETURN_ON_FAIL_BOOL(DescriptionReaderLog, !Descriptions_.Items.IsEmpty());
		
		PlayFab::ServerModels::FSetTitleDataRequest Request;
		
		Request.Key = GetName();
		FString JsonString;
		bool bOk = FJsonObjectConverter::UStructToJsonObjectString(Descriptions_, JsonString, 0, 0, 0, nullptr, false);
		RETURN_ON_FAIL_BOOL_T(DescriptionReaderLog, bOk, TEXT("Failed to JSON convert"));
		
		Request.Value = JsonString;

		PlayFab::UPlayFabServerAPI::FSetTitleDataDelegate SuccessDelegate;
		SuccessDelegate.BindWeakLambda(InUserObject, [](const PlayFab::ServerModels::FSetTitleDataResult& InResult)
		{
			MG_LOG(DescriptionReaderLog, TEXT("PlayFab login success!"));
		});

		PlayFab::FPlayFabErrorDelegate ErrorDelegate;
		ErrorDelegate.BindWeakLambda(InUserObject, [](const PlayFab::FPlayFabCppError& InError)
		{
			MG_ERROR(DescriptionReaderLog, TEXT("PlayFab login failed: %s"), *InError.GenerateErrorReport());
		});
				
		bOk = InPlayFabAPI->SetTitleData(Request, SuccessDelegate, ErrorDelegate);
		MG_COND_ERROR(DescriptionReaderLog, !bOk, TEXT("InPlayFabAPI->SetTitleData was failed!"));
		return bOk; 
	};

	virtual bool ReadFromPlayFab(const PlayFabServerPtr& InPlayFabAPI, UWarpPlayfabContentSubSystem* InUserObject) override
	{
		PlayFab::ServerModels::FGetTitleDataRequest Request;
		
		Request.Keys = { GetName() };

		PlayFab::UPlayFabServerAPI::FGetTitleDataDelegate SuccessDelegate;

		SuccessDelegate.BindWeakLambda(InUserObject, [this](const PlayFab::ServerModels::FGetTitleDataResult& InResult)
		{
			const FString Key = GetName();
			const FString* JsonPtr = InResult.Data.Find(Key);

			if (!JsonPtr || JsonPtr->IsEmpty())
			{
				MG_ERROR(DescriptionReaderLog, TEXT("TitleData key '%s' not found or empty."), *Key);
				return;
			}
				
		});
		
		PlayFab::FPlayFabErrorDelegate ErrorDelegate;
		ErrorDelegate.BindWeakLambda(InUserObject, [](const PlayFab::FPlayFabCppError& InError)
		{
			MG_ERROR(DescriptionReaderLog, TEXT("GetTitleData failed: %s"), *InError.GenerateErrorReport());
		});
		
		return true;
	};

	virtual void UpdateDescriptionVersion() override
	{
		++Descriptions_.Version;
	};

	virtual void UpdateVersions(const FString& /*InDescriptionName*/, int32 /*InVersion*/) override
	{
		ensureMsgf(false, TEXT("UpdateVersions called for non-version struct %s"), *GetName());
	}

	
	
	TUStruct Descriptions_;
};


template<>
inline void FUStructDescriptionReader<FDescriptionVersions>::UpdateVersions(
	const FString& InDescriptionName,
	int32 InVersion)
{
	++Descriptions_.Version;

	if (FDescriptionVersion* Existing = Descriptions_.Items.FindByPredicate(
		[&](const FDescriptionVersion& It) { return It.DescriptionName == InDescriptionName; }))
	{
		Existing->Version = InVersion;
	}
	else
	{
		FDescriptionVersion& NewItem = Descriptions_.Items.AddDefaulted_GetRef();
		NewItem.DescriptionName = InDescriptionName;
		NewItem.Version   = InVersion;
	}
}