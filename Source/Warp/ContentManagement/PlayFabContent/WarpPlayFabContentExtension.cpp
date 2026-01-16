
#include "WarpPlayFabContentExtension.h"

#include "DescriptionReader.hpp"
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
}
