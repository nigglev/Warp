#pragma once
#include "CoreMinimal.h"
#include "UnitCatalogDTO.generated.h"

USTRUCT(BlueprintType)
struct FUnitRecordDTO
{
	GENERATED_BODY()

	UPROPERTY()
	FName UnitName;

	UPROPERTY()
	FString UnitSize;

	UPROPERTY()
	int32 UnitSpeed = 0;

	UPROPERTY()
	int32 UnitMaxAP = 0;

	UPROPERTY()
	FString MeshPath;

	UPROPERTY()
	TArray<FString> Tags; 
};