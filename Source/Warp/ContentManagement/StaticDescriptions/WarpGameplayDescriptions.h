// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "JsonObjectConverter.h"
#include "WarpDescriptionBase.h"
#include "UObject/Object.h"
#include "WarpGameplayDescriptions.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGameplayDescription : public FBaseDescription
{
	GENERATED_BODY()

	static inline const FName DescrName = TEXT("GameplayDescriptions");
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<FName> DefaultPlayerUnitTypes = 
	{
		TEXT("corvette"), TEXT("corvette_black"), TEXT("corvette_blue"), TEXT("corvette_green")
	};

	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CampaignMapLayerCount = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CampaignMapNodeInLayerCountMin = 3;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 CampaignMapNodeInLayerCountMax = 4;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) int32 ShipsOnStart = 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite) float GhostDelayTime = 1;
};

USTRUCT(BlueprintType)
struct FGameplayDescriptions : public FBaseDescriptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FGameplayDescription> Items;

	virtual bool AreItemsEmpty() override
	{
		return Items.IsEmpty();
	}

	virtual void EmplaceNewItem() override
	{
		Items.Emplace();
	}

	virtual FBaseDescription* Find(FName InDescrName) override
	{
		return Items.FindByPredicate([InDescrName](const FGameplayDescription& InDescr) { return InDescr.Name == InDescrName; } );
	}
	
	virtual int32 Num() const override { return Items.Num(); }
	
	virtual FBaseDescription* At(int32 InIndex) override
	{
		if (!Items.IsValidIndex(InIndex))
			return nullptr;
		
		return &Items[InIndex];
	}
	
	virtual const FBaseDescription* At(int32 InIndex) const override
	{
		if (!Items.IsValidIndex(InIndex))
			return nullptr;
		
		return &Items[InIndex];
	}
	
	virtual bool JsonToDescription(const FString& InJsonString, FText* OutFailReason) override
	{
		if (!ensure(!InJsonString.IsEmpty()))
			return false;
		const bool bOk = FJsonObjectConverter::JsonObjectStringToUStruct(InJsonString, this,0,0,false, OutFailReason);
		return bOk;
	}

	virtual bool DescriptionToJson(FString& OutJsonString) override
	{
		OutJsonString.Empty();
		
		bool bOk = FJsonObjectConverter::UStructToJsonObjectString(*this, OutJsonString, 0, 0, 0, nullptr, true);
		return bOk;
	}
};