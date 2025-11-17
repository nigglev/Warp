#pragma once
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/AssetManager.h"
#include "Warp/CombatMap/CombatMap.h"
#include "PlayFabUnitTypes.generated.h"

USTRUCT()
struct FPlayFabUnitDisplayProps
{
	GENERATED_BODY()

	UPROPERTY()
	FString UnitSize;

	UPROPERTY()
	int32 UnitSpeed = 0;

	UPROPERTY()
	int32 UnitMaxAP = 0;
	
	UPROPERTY()
	FSoftObjectPath MeshPath;

	UPROPERTY()
	TArray<FString> Tags;
};

USTRUCT()
struct FUnitRecord
{
	GENERATED_BODY()
	
	UPROPERTY()
	FName UnitName;

	UPROPERTY()
	FPlayFabUnitDisplayProps Props;

	EUnitSizeCategory GetSizeCategory() const
	{
		if (Props.UnitSize == FName(TEXT("Small")))
			return EUnitSizeCategory::Small;

		if (Props.UnitSize == FName(TEXT("Medium")))
			return EUnitSizeCategory::Medium;

		if (Props.UnitSize == FName(TEXT("Big")))
			return EUnitSizeCategory::Big;

		return EUnitSizeCategory::None;
	}
	
};