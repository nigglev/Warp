#pragma once

#include "CoreMinimal.h"
#include "HullSize.generated.h"

USTRUCT()
struct FHullSize
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	uint8 Forward = 0;
	
	UPROPERTY(EditAnywhere)
	uint8 Backward = 0;
	
	UPROPERTY(EditAnywhere)
	uint8 Left = 0;
	
	UPROPERTY(EditAnywhere)
	uint8 Right = 0;
};