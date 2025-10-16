// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Warp/Units/UnitBase.h"
#include "UnitStaticData.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FUnitStaticData : public FTableRowBase
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName UnitName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	EUnitSizeCategory UnitSize;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UnitSpeed;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 UnitMaxAP;
	
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UStaticMesh> Mesh;
};