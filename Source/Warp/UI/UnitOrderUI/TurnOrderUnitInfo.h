// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnOrderUnitInfo.generated.h"

USTRUCT()
struct FTurnOrderUnitInfo
{
	GENERATED_BODY()
	
	uint32 UnitCombatId_ = INDEX_NONE;
	
	FName UnitTypeName_;
	
	TObjectPtr<class UTexture2D> Icon_ = nullptr;
	
	bool bIsAlly_ = false;
};
