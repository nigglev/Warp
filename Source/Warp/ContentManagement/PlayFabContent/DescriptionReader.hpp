// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DescriptionReaderBase.h"
#include "UObject/Object.h"
#include "JsonObjectConverter.h"
#include "MGLogs.h"
#include "Core/PlayFabServerAPI.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayFabContentExtension.h"

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

	virtual bool ReadGameplaySource(const FAnyPlayFabPtr& InApi) override
	{
		FString RelativeContentDirectory = FPaths::ProjectContentDir();
		const FString FileName = FString::Printf(TEXT("%s.json"), *GetName());

		FString Filepath;
		if (InApi.IsType<PlayFabServerPtr>())
		{
			Filepath = FPaths::Combine(FPaths::ProjectDir(), TEXT("GameDataSource"), FileName);
		}
		else
		{
			Filepath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"), FileName);
		}
		
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

	virtual bool WriteGameplaySource(const FAnyPlayFabPtr& InApi) override
	{
		FString RelativeContentDirectory = FPaths::ProjectContentDir();
		const FString FileName = FString::Printf(TEXT("%s.json"), *GetName());

		FString Filepath;
		if (InApi.IsType<PlayFabServerPtr>())
		{
			Filepath = FPaths::Combine(FPaths::ProjectDir(), TEXT("GameDataSource"), FileName);
		}
		else
		{
			Filepath = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("GameDataSource"), FileName);
		}
		
		FString JsonString;
		bool bOk = FJsonObjectConverter::UStructToJsonObjectString(Descriptions_, JsonString, 0, 0, 0, nullptr, false);
		RETURN_ON_FAIL_BOOL_T(DescriptionReaderLog, bOk, TEXT("Failed to JSON convert"));
		
		if (!FFileHelper::SaveStringToFile(JsonString, *Filepath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			MG_COND_ERROR(DescriptionReaderLog, !bOk, TEXT("Failed to write to file: %s"), *Filepath);
		}

		return bOk;
	}
	

	virtual bool ReadFromPlayFab(const FAnyPlayFabPtr& InApi, UReaderObserver* InUserObject) override
	{
		return Visit([this, InUserObject](const auto& ApiPtr) {
			
		  using ApiPtrType = std::decay_t<decltype(ApiPtr)>;

		  if constexpr (std::is_same_v<ApiPtrType, PlayFabServerPtr>)
		  {
			  return ReadFromPlayFabImpl<WarpPlayfabContent::FServerTag>(ApiPtr, InUserObject);
		  }
		  else
		  {
			  return ReadFromPlayFabImpl<WarpPlayfabContent::FClientTag>(ApiPtr, InUserObject);
		  }
	   }, InApi);
	}

	// virtual void UpdateDescriptionVersion() override
	// {
	// 	++Descriptions_.Version;
	// };

	virtual TUStruct& GetDescriptions()
	{
		return Descriptions_;
	}

	virtual const TUStruct& GetDescriptions() const
	{
		return Descriptions_;
	}
	
	TUStruct Descriptions_;

private:
	template<typename TTag>
	bool ReadFromPlayFabImpl(const TSharedPtr<typename TTag::TPlayFabAPI>& InPlayFabAPI, UReaderObserver* InUserObject)
	{
		RETURN_ON_FAIL_BOOL(WarpPlayfabContentLog, InPlayFabAPI != nullptr);
		RETURN_ON_FAIL_BOOL(WarpPlayfabContentLog, InUserObject != nullptr);

		typename TTag::TGetTitleDataRequest Request;
		Request.Keys = { GetName() };

		typename TTag::TGetTitleDataDelegate SuccessDelegate;
		SuccessDelegate.BindWeakLambda(InUserObject,
			[this, InUserObject](const typename TTag::TGetTitleDataResult& InResult)
		{
			const FString Key = GetName();
			const FString* Data = InResult.Data.Find(Key);

			if (!Data)
			{
				MG_ERROR(DescriptionReaderLog, TEXT("GetTitleData: key '%s' not found"), *Key);
				InUserObject->OnDescriptionReadingResult(this, false);
				return;
			}

			const bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct(*Data, &Descriptions_);
			if (!bParsed)
			{
				MG_ERROR(DescriptionReaderLog, TEXT("GetTitleData: JSON->UStruct failed for key '%s'"), *Key);
			}

			InUserObject->OnDescriptionReadingResult(this, bParsed);
		});

		PlayFab::FPlayFabErrorDelegate ErrorDelegate;
		ErrorDelegate.BindWeakLambda(InUserObject, [this, InUserObject](const PlayFab::FPlayFabCppError& InError)
		{
			MG_ERROR(DescriptionReaderLog, TEXT("GetTitleData failed: %s"), *InError.GenerateErrorReport());
			InUserObject->OnDescriptionReadingResult(this, false);
		});

		const bool bOk = InPlayFabAPI->GetTitleData(Request, SuccessDelegate, ErrorDelegate);
		if (!bOk)
		{
			InUserObject->OnDescriptionReadingResult(this, false);
		}
		return bOk;
	}
	
};
