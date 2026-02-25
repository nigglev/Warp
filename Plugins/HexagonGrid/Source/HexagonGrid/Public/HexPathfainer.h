#pragma once
#include "CoreMinimal.h"
#include "HexMath.h"

namespace HexMath
{
	struct HEXAGONGRID_API FPathNode
	{
		FAxialCoord Coord;
		int8 Rotation = 0;
		float Distance = TNumericLimits<float>::Max();

		FPathNode() = default;
		FPathNode(const FAxialCoord& InCoord, int8 InRotation) : Coord(InCoord), Rotation(InRotation) {}
		FPathNode(const FAxialCoord& InCoord, int8 InRotation, float InDistance) : Coord(InCoord), Rotation(InRotation), Distance(InDistance) {}
		
		FString ToString() const;
	};
	
	FORCEINLINE uint32 GetTypeHash(const FPathNode& Key)
	{
		return GetTypeHash(Key.Coord);
	}
	
	inline bool operator==(const FPathNode& LHS, const FPathNode& RHS) { return LHS.Coord == RHS.Coord; }
	inline bool operator!=(const FPathNode& LHS, const FPathNode& RHS) { return LHS.Coord != RHS.Coord; }

	void FindPathZone(const FAxialCoord& InStart, int8 InStartRotation, float InMaxDistance,
		TSet<FPathNode>& OutPath, float InMoveCost, float InRotationCost, bool InLog);
	
	bool FindPath(const FAxialCoord& InStart, int8 InStartRotation, const FAxialCoord& InEnd, TOptional<int8> InEndRotation, 
		float InMaxDistance, TArray<FPathNode>& OutPath, float InMoveCost, float InRotationCost, bool InLog);
}