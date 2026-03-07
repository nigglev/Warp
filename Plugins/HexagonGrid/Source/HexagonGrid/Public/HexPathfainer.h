#pragma once
#include "CoreMinimal.h"
#include "HexMath.h"
#include "HullHexFootprint.h"
#include "MoveParams.h"

namespace HexMath
{
	HEXAGONGRID_API int8 GetRotationDiff(int8 A, int8 B);
	
	struct HEXAGONGRID_API FPathNode
	{
		FAxialCoord Coord;
		int8 Rotation = 0;
		float Distance = TNumericLimits<float>::Max();

		FPathNode() = default;
		FPathNode(const FAxialCoord& InCoord, int8 InRotation) : Coord(InCoord), Rotation(InRotation) {}
		FPathNode(const FAxialCoord& InCoord, int8 InRotation, float InDistance) : Coord(InCoord), Rotation(InRotation), Distance(InDistance) {}
		
		FString ToString() const;
		FString ToDebugScreenString() const;
	};
	
	FORCEINLINE uint32 GetTypeHash(const FPathNode& Key)
	{
		return GetTypeHash(Key.Coord);
	}
	
	inline bool operator==(const FPathNode& LHS, const FPathNode& RHS) { return LHS.Coord == RHS.Coord; }
	inline bool operator!=(const FPathNode& LHS, const FPathNode& RHS) { return LHS.Coord != RHS.Coord; }
	
	void CaptureCells(const FAxialCoord& InCenter, int8 InRotation, const FHullHexFootprint& InHullSize, TArray<FAxialCoord>& OutCells, bool InLog);

	void FindPathZone(const FAxialCoord& InStart, int8 InStartRotation, const FMoveParams& InMoveParams,
		TSet<FPathNode>& OutPath, bool InLog);
	
	bool FindPath(const FAxialCoord& InStart, int8 InStartRotation, const FAxialCoord& InEnd, TOptional<int8> InEndRotation, 
		const FMoveParams& InMoveParams, TArray<FPathNode>& OutPath, bool InLog);
}
