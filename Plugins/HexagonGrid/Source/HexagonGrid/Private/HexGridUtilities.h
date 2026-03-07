#pragma once
#include "HexMath.h"

class AHexGridISMActor;

namespace HexGridUtilities
{
	TOptional<HexMath::FOffsetCoord> WorldToChunkCoord(const FVector& InWorldPoint);
	TOptional<HexMath::FAxialCoord> WorldToAxialCellCoord(const FVector& InWorldPoint);
	
	struct FHexGridActorCDODataCache
	{
		float HexSize = 0;
		int32 NumColsRows = 0; //размеры чанка в ячейках
		int32 BuildChunkAround = 0;
		int32 SelectRadius = 1;
		bool PathfinderLog = false;
		
		TSubclassOf<AHexGridISMActor> HexGridActorClass_ = nullptr;
	};
	TOptional<FHexGridActorCDODataCache> GetHexGridActorCDODataCache();
	
	uint32 GetColRowCountInChunk();
};
