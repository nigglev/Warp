#pragma once
#include "CoreMinimal.h"
#include "HexMath.h"

namespace HexMath
{
	struct FWaveElem
	{
		HexMath::FAxialCoord Coord;
		int8 Rotation;
		uint32 Distance;
		
		FString ToString() const;
	};
	
	FORCEINLINE uint32 GetTypeHash(const FWaveElem& Key)
	{
		return GetTypeHash(Key.Coord);
	}
	
	inline bool operator==(const FWaveElem& LHS, const FWaveElem& RHS) { return LHS.Coord == RHS.Coord; }
	inline bool operator!=(const FWaveElem& LHS, const FWaveElem& RHS) { return LHS.Coord != RHS.Coord; }

	void FindPathZone(const HexMath::FAxialCoord& InStart, int8 InStartRotation, uint32 InMaxWave,
		TSet<FWaveElem>& OutPath, bool InLog /*= false*/);
}