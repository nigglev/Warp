
#include "WarpPlayFabContentExtension.h"

#include "DescriptionReader.hpp"
#include "WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameplayDescriptions.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"
DEFINE_LOG_CATEGORY_STATIC(AWarpPlayfabContentExt, Log, All);

namespace WarpPlayfabContent
{
	static TMap<FName, FReaderFactory>& GetReaderFactories()
	{
		static TMap<FName, FReaderFactory> ReaderFactories = {
			{ TEXT("UnitDescriptions"), [](){ return MakeUnique<FUStructDescriptionReader<FUnitDescriptions>>(); } },
{			 TEXT("GameplayDescriptions"), [](){ return MakeUnique<FUStructDescriptionReader<FGameplayDescriptions>>(); } },
		};
		return ReaderFactories;
	}
	
	TUniquePtr<FDescriptionReaderBase> CreateReaderByKey(const FName& Key)
	{
		TMap<FName, FReaderFactory>& Factories = GetReaderFactories();
		if (const FReaderFactory* Factory = Factories.Find(Key))
		{
			return (*Factory)();
		}
		return nullptr;
	}
	
	TOptional<FString> ReadSecret()
	{
		const FString PlayfabKeysPath(TEXT("PlayfabKeys"));
		const FString PathValue = FPlatformMisc::GetEnvironmentVariable(*PlayfabKeysPath);

		FString Secret;
		if (GConfig->GetString(TEXT("PlayFab"), TEXT("SecretKey"), Secret, PathValue))
		{
			Secret.TrimStartAndEndInline();
			Secret.ReplaceInline(TEXT("\""), TEXT(""));

			if (!Secret.IsEmpty())
			{
				return TOptional<FString>(MoveTemp(Secret)); // UE-style move
			}
		}

		return {};
	}

	bool SaveDescriptionToPlayFab(const PlayFabServerPtr& InPlayFabAPI, const FString& InKey, const FString& InJsonToSave, UWarpPlayfabContentSubSystem* InContentSubSystem)
	{
		RETURN_ON_FAIL_BOOL(DescriptionReaderLog, InPlayFabAPI != nullptr);
		RETURN_ON_FAIL_BOOL(DescriptionReaderLog, !InJsonToSave.IsEmpty());
		RETURN_ON_FAIL_BOOL(DescriptionReaderLog, InContentSubSystem != nullptr);
	
		PlayFab::ServerModels::FSetTitleDataRequest Request;
		Request.Key = InKey;
		Request.Value = InJsonToSave;

		PlayFab::UPlayFabServerAPI::FSetTitleDataDelegate SuccessDelegate;
		SuccessDelegate.BindWeakLambda(InContentSubSystem, [InContentSubSystem](const PlayFab::ServerModels::FSetTitleDataResult& InResult)
		{
			//InContentSubSystem->OnDescriptionSavingResult(true);
			MG_LOG(DescriptionReaderLog, TEXT("PlayFab login success!"));
		});

		PlayFab::FPlayFabErrorDelegate ErrorDelegate;
		ErrorDelegate.BindWeakLambda(InContentSubSystem, [InContentSubSystem](const PlayFab::FPlayFabCppError& InError)
		{
			//InContentSubSystem->OnDescriptionSavingResult(false);
			MG_ERROR(AWarpPlayfabContentExt, TEXT("PlayFab login failed: %s"), *InError.GenerateErrorReport());
		});

		bool bOk = InPlayFabAPI->SetTitleData(Request, SuccessDelegate, ErrorDelegate);
		MG_COND_ERROR(AWarpPlayfabContentExt, !bOk, TEXT("InPlayFabAPI->SetTitleData was failed!"));

		return bOk;
	}

	// bool DownloadVersionsFromPlayFab(const PlayFabClientPtr& InPlayFabAPI, FDescriptionVersions& OutVersions)
	// {
	// 	RETURN_ON_FAIL_BOOL(DescriptionReaderLog, InPlayFabAPI != nullptr);
	// 	
	// 	PlayFab::ClientModels::FGetTitleDataRequest Request;
	// 	Request.Keys.Add(TEXT("DescriptionVersions"));
	//
	// 	const bool bOk = InPlayFabAPI->GetTitleData(Request,
	// 		PlayFab::UPlayFabClientAPI::FGetTitleDataDelegate::CreateLambda([&OutVersions](const PlayFab::ClientModels::FGetTitleDataResult& Result)
	// 			{
	// 			  const FString* Value = Result.Data.Find(TEXT("DescriptionVersions"));
	// 			  if (!Value)
	// 			  {
	// 				  MG_ERROR(AWarpPlayfabContentExt, TEXT("GetTitleData: DescriptionVersions not found"));
	// 				  return;
	// 			  }
	//
	// 			  FDescriptionVersions Versions;
	// 			  const bool bParsed = FJsonObjectConverter::JsonObjectStringToUStruct(*Value, &OutVersions);
	// 			  if (!bParsed)
	// 			  {
	// 				  MG_ERROR(AWarpPlayfabContentExt, TEXT("GetTitleData: failed to parse DescriptionVersions JSON: %s"), **Value);
	// 				  return;
	// 			  }
	// 			}
	// 	),
	// 	PlayFab::FPlayFabErrorDelegate::CreateLambda([](const PlayFab::FPlayFabCppError& ErrorResult)
	// 		{
	// 		  MG_ERROR(AWarpPlayfabContentExt, TEXT("GetTitleData failed: %s"), *ErrorResult.GenerateErrorReport());
	// 		}
	// 	));
	//
	// 	return bOk;
	// }
}
