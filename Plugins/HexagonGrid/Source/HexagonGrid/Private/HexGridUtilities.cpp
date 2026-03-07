#include "HexGridUtilities.h"

#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"

TOptional<HexGridUtilities::FHexGridActorCDODataCache> HexGridUtilities::GetHexGridActorCDODataCache()
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (!ensure(Settings->HexGridActorClass_ != nullptr))
		return {};
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (!ensure(CDO != nullptr))
		return {};

	FHexGridActorCDODataCache Cache;
	Cache.HexSize = CDO->GetHexSize();
	Cache.NumColsRows = CDO->GetGridSize();
	Cache.BuildChunkAround = Settings->BuildChunkAround;
	Cache.HexGridActorClass_ = Settings->HexGridActorClass_;
	Cache.SelectRadius = Settings->SelectRadius;
	Cache.PathfinderLog = Settings->PathfinderLog;

	return Cache;
}

TOptional<HexMath::FOffsetCoord> HexGridUtilities::WorldToChunkCoord(const FVector& InWorldPoint)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return {};
	
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CacheOpt->HexSize;
	int32 NumColsRows = CacheOpt->NumColsRows;
	
	FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(InWorldPoint, HexSize);
	
	FOffsetCoord OCoord = HexMath::HexMathAxial::WorldToOffset<HEX_LAYOUT>(Coord, HexSize);
	
	FOffsetCoord ChunkCoord = OffsetCellToChunk(OCoord, NumColsRows, NumColsRows);
	
	return ChunkCoord;
}

TOptional<HexMath::FAxialCoord> HexGridUtilities::WorldToAxialCellCoord(const FVector& InWorldPoint)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return {};
	
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CacheOpt->HexSize;
	
	FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(InWorldPoint, HexSize);
	
	FAxialCoord CellCoord = HexMathAxial::OffsetToAxial<HEX_LAYOUT>(Coord, HexSize);
	
	return CellCoord;
}

uint32 HexGridUtilities::GetColRowCountInChunk()
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return 3;
	return CacheOpt->NumColsRows;
}