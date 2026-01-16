#pragma once

#include "CoreMinimal.h"
#include "EUnitSize.h"
#include "JsonObjectConverter.h"

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
		FDescriptionVersion* Item = Items.FindByPredicate([InDescriptionName](const FDescriptionVersion& It){return It.DescriptionName == InDescriptionName;});
		if (!ensure(Item != nullptr))
			return;
		
		Item->Version = InVersion;

		int32 Sum = 0;
		for (const FDescriptionVersion& It : Items)
		{
			Sum += FMath::Max(0, It.Version);
		}

		Version += Sum;
		VersionDate = FDateTime::UtcNow();
	};

	
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FDateTime VersionDate = FDateTime::UtcNow();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FDescriptionVersion> Items;	
};

USTRUCT(BlueprintType)
struct FBaseDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = TEXT("TestName");
};

USTRUCT(BlueprintType)
struct FUnitDescription : public FBaseDescription
{
	GENERATED_BODY()

	static inline const FName DescrName = TEXT("UnitDescriptions");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EUnitSize UnitSize = EUnitSize::Medium;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitSpeed = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 UnitMaxAP = 2;
};

USTRUCT(BlueprintType)
struct FBaseDescriptions
{
	GENERATED_BODY()
	virtual ~FBaseDescriptions() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;

	virtual FBaseDescription* Find(FName InDescrName) PURE_VIRTUAL(FBaseDescriptions::Find, return nullptr;);
	virtual void JsonToDescription(const FString& InJsonString) PURE_VIRTUAL(FBaseDescriptions::JsonToDescription, );
};

USTRUCT(BlueprintType)
struct FUnitDescriptions : public FBaseDescriptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FUnitDescription> Items;

	virtual FBaseDescription* Find(FName InDescrName) override
	{
		return Items.FindByPredicate([InDescrName](const FUnitDescription& InDescr) { return InDescr.Name == InDescrName; } );
	}
	
	virtual void JsonToDescription(const FString& InJsonString) override
	{
		if (!ensure(!InJsonString.IsEmpty()))
			return;
		const bool bOk = FJsonObjectConverter::JsonObjectStringToUStruct(InJsonString, this);
	}
};