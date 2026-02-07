// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "UObject/Object.h"
#include "WarpGameVersion.generated.h"

USTRUCT(BlueprintType)
struct FDescriptionVersion
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString DescriptionName = TEXT("DefaultDescription");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;
};

USTRUCT(BlueprintType)
struct FDescriptionVersions
{
	GENERATED_BODY()

	void UpdateVersions(const FString& InDescriptionName, int32 InVersion)
	{
		FDescriptionVersion* Item = Items.FindByPredicate([InDescriptionName](const FDescriptionVersion& It){return It.DescriptionName == InDescriptionName;});
		if (!ensure(Item != nullptr))
			return;
		
		Item->Version = InVersion;

		UpdateVersion();
	};

	void UpdateVersion()
	{
		int32 Sum = 0;
		for (const FDescriptionVersion& It : Items)
		{
			Sum += FMath::Max(0, It.Version);
		}

		Version += Sum;
		VersionDate = FDateTime::UtcNow();
	}

	bool VersionsToJson(FString& OutJsonString) const
	{
		OutJsonString.Empty();
		
		bool bOk = FJsonObjectConverter::UStructToJsonObjectString(*this, OutJsonString, 0, 0, 0, nullptr, true);
		return bOk;
	}
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime VersionDate = FDateTime::UtcNow();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDescriptionVersion> Items;	
};
