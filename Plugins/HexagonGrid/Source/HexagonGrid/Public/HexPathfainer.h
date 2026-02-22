#pragma once
#include "CoreMinimal.h"
#include "HexMath.h"

namespace HexMath
{
	struct FWaveElem
	{
		FAxialCoord Coord;
		int8 Rotation;
		float Distance;
		
		FString ToString() const;
	};
	
	FORCEINLINE uint32 GetTypeHash(const FWaveElem& Key)
	{
		return GetTypeHash(Key.Coord);
	}
	
	inline bool operator==(const FWaveElem& LHS, const FWaveElem& RHS) { return LHS.Coord == RHS.Coord; }
	inline bool operator!=(const FWaveElem& LHS, const FWaveElem& RHS) { return LHS.Coord != RHS.Coord; }

	void FindPathZone(const HexMath::FAxialCoord& InStart, int8 InStartRotation, float InMaxWave,
		TSet<FWaveElem>& OutPath, FVector2D InStepRotationPrice, bool InLog);
}