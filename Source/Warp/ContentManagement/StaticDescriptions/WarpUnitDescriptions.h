// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CannonParams.h"
#include "HullHexFootprint.h"
#include "JsonObjectConverter.h"
#include "MoveParams.h"
#include "WarpDescriptionBase.h"
#include "UObject/Object.h"
#include "WarpUnitDescriptions.generated.h"

USTRUCT()
struct FUnitDescription : public FBaseDescription
{
	GENERATED_BODY()

	static inline const FName DescrName = TEXT("UnitDescriptions");
	
	UPROPERTY(EditAnywhere)
	FHullHexFootprint Footprint;
	
	UPROPERTY(EditAnywhere)
	FMoveParams MoveParams;
	
	UPROPERTY(EditAnywhere)
	float AnimationMoveSpeed = 600;
	
	UPROPERTY(EditAnywhere)
	float AnimationRotationSpeed = 3600;
	
	UPROPERTY(EditAnywhere)
	float MaxHealth = 100;
	
	UPROPERTY(EditAnywhere)
	int32 MovePriority = 10;
	
	UPROPERTY(EditAnywhere)
	TArray<FCannonParams> CannonParams;
};

USTRUCT(BlueprintType)
struct FUnitDescriptions : public FBaseDescriptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FUnitDescription> Items;

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
		return Items.FindByPredicate([InDescrName](const FUnitDescription& InDescr) { return InDescr.Name == InDescrName; } );
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
