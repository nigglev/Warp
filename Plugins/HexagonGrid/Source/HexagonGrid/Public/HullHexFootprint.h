#pragma once

#include "CoreMinimal.h"
#include "HullHexFootprint.generated.h"

USTRUCT()
struct FHullHexFootprint
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TArray<FIntVector2> Cells;
};