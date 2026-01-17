
#include "WarpPlayFabContentExtension.h"

#include "DescriptionReader.hpp"
#include "WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/UnitDescription.h"

namespace WarpPlayfabContent
{
	static TMap<FName, FReaderFactory>& GetReaderFactories()
	{
		static TMap<FName, FReaderFactory> ReaderFactories = {
			{ TEXT("UnitDescriptions"), [](){ return MakeUnique<FUStructDescriptionReader<FUnitDescriptions>>(); } },
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
			InContentSubSystem->OnDescriptionSavingResult(true);
			MG_LOG(DescriptionReaderLog, TEXT("PlayFab login success!"));
		});

		PlayFab::FPlayFabErrorDelegate ErrorDelegate;
		ErrorDelegate.BindWeakLambda(InContentSubSystem, [InContentSubSystem](const PlayFab::FPlayFabCppError& InError)
		{
			InContentSubSystem->OnDescriptionSavingResult(false);
			MG_ERROR(DescriptionReaderLog, TEXT("PlayFab login failed: %s"), *InError.GenerateErrorReport());
		});

		bool bOk = InPlayFabAPI->SetTitleData(Request, SuccessDelegate, ErrorDelegate);
		MG_COND_ERROR(DescriptionReaderLog, !bOk, TEXT("InPlayFabAPI->SetTitleData was failed!"));

		return bOk;
	}
}
