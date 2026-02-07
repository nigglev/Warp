// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WarpDescriptionBase.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FBaseDescription
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName Name = TEXT("TestName");
};

USTRUCT(BlueprintType)
struct FBaseDescriptions
{
	GENERATED_BODY()
	virtual ~FBaseDescriptions() = default;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Version = 0;

	virtual void EmplaceNewItem() PURE_VIRTUAL(FBaseDescriptions::EmplaceNewItem);
	virtual bool AreItemsEmpty() PURE_VIRTUAL(FBaseDescriptions::AreItemsEmpty, return true;);
	
	virtual FBaseDescription* Find(FName InDescrName) PURE_VIRTUAL(FBaseDescriptions::Find, return nullptr;);
	
	virtual int32 Num() const PURE_VIRTUAL(FBaseDescriptions::Num, return INDEX_NONE;);
	virtual FBaseDescription* At(int32 InIndex) PURE_VIRTUAL(FBaseDescriptions::At, return nullptr;);
	virtual const FBaseDescription* At(int32 InIndex) const PURE_VIRTUAL(FBaseDescriptions::At, return nullptr;);
	
	virtual bool JsonToDescription(const FString& InJsonString, FText* OutFailReason) PURE_VIRTUAL(FBaseDescriptions::JsonToDescription, return false;);
	virtual bool DescriptionToJson(FString& OutJsonString) PURE_VIRTUAL(FBaseDescriptions::JsonToDescription, return false;);
};
