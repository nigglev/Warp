#pragma once

#include "CoreMinimal.h"
#include "EUnitSize.h"

#include "UnitDescription.generated.h"

USTRUCT(BlueprintType)
struct FUnitDescription
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName UnitTypeName = TEXT("TestUnit");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUnitSize UnitSize = EUnitSize::Medium;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitSpeed = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitMaxAP = 2;
};

USTRUCT(BlueprintType)
struct FUnitDescriptions
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUnitDescription> Items;	
};