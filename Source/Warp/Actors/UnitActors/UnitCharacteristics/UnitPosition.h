#pragma once

#include "CoreMinimal.h"
#include "UnitPosition.generated.h"

USTRUCT()
struct FUnitPosition
{
	GENERATED_BODY()

	FIntVector2 GetUnitTilePosition() const {return FIntVector2(UnitTileX, UnitTileY);}
	FVector2f GetUnitWorldPosition() const { return UnitWorldPosition; }

	void SetUnitPosition(const FVector2f& InWorldPosition, const FIntVector2& InTilePosition) {UnitWorldPosition = InWorldPosition; UnitTileX = InTilePosition.X; UnitTileY = InTilePosition.Y;}
	void SetUnitTilePosition(const FIntVector2& InTilePosition) {UnitTileX = InTilePosition.X; UnitTileY = InTilePosition.Y;}
	void SetUnitWorldPosition(const FVector2f& InWorldPosition) {UnitWorldPosition = InWorldPosition;}
	
private:	
	UPROPERTY()
	uint32 UnitTileX = UINT32_MAX;
	UPROPERTY()
	uint32 UnitTileY = UINT32_MAX;
	UPROPERTY()
	FVector2f UnitWorldPosition = FVector2f::ZeroVector;
};