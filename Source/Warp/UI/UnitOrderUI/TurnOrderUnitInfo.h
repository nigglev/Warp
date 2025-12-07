// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TurnOrderUnitInfo.generated.h"

USTRUCT()
struct FTurnOrderUnitInfo
{
	GENERATED_BODY()
	
	FName UnitTypeName_;
	bool bIsAlly_ = false;
};
