#pragma once

#include "CoreMinimal.h"
#include "EUnitSize.h"

#include "UnitDescription.generated.h"

USTRUCT(BlueprintType)
struct FDescriptionVersion
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DescriptionName = TEXT("UnitDescription");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;
};

USTRUCT(BlueprintType)
struct FDescriptionVersions
{
	GENERATED_BODY()

	void UpdateVersions(const FString& InDescriptionName, int32 InVersion)
	{
		++Version;
		FDescriptionVersion* Item = Items.FindByPredicate([InDescriptionName](const FDescriptionVersion& It){return It.DescriptionName == InDescriptionName;});
		RETURN_ON_FAIL(DescriptionReaderLog, Item);
		Item->Version = InVersion;
	};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDescriptionVersion> Items;	
};

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
	int32 Version = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUnitDescription> Items;	
};