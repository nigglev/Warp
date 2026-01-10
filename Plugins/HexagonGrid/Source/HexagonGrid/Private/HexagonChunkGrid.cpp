// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonChunkGrid.h"
#include "HexMath.h"
#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridLog, Log, Log);

#define HEX_LAYOUT HexMath::EHexOffsetLayout::FlatTopOddQ

void UHexagonChunkGrid::OnChangeObserverPosition(const FVector& InNewPosition)
{
	CreateNewChunks(InNewPosition);	
}

TOptional<UHexagonChunkGrid::FHexGridActorCDODataCache> UHexagonChunkGrid::GetHexGridActorCDODataCache()
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
	
	return Cache;
}

void UHexagonChunkGrid::CreateNewChunks(const FVector& InNewPosition)
{
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	TOptional<FOffsetCoord> ObserverChunkCoordOp = WorldToChunkCoord(InNewPosition);
	if (!ObserverChunkCoordOp.IsSet())
		return;
	
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	float HexSize = CacheOpt->HexSize;
	uint32 NumColsRows = CacheOpt->NumColsRows;

	FOffsetRealCoord ChunkPitch = GetChunkPitchNoGap<HEX_LAYOUT>(NumColsRows, NumColsRows, HexSize);
	
	FOffsetCoord ObserverChunkCoord = ObserverChunkCoordOp.GetValue();
	
	if (CurrentChunkCoord_ != ObserverChunkCoord)
	{
		CurrentChunkCoord_ = ObserverChunkCoord;
		
		uint32 iT = CacheOpt->BuildChunkAround;
		
		for (int64 Idx = CurrentChunkCoord_.Up - iT; Idx <= CurrentChunkCoord_.Up + iT; ++Idx)
			for (int64 Jdx = CurrentChunkCoord_.Right - iT; Jdx <= CurrentChunkCoord_.Right + iT; ++Jdx)
			{
				FOffsetCoord ChunkCoord(Jdx, Idx);
				int32 ChunkIndex = FindChunkIndex(ChunkCoord);
				bool bAlreadyPresent = ChunkIndex != INDEX_NONE;
				
				if (!bAlreadyPresent)
				{
					FOffsetRealCoord NewChunkPos(ChunkCoord.Right * ChunkPitch.Right,  ChunkCoord.Up * ChunkPitch.Up);
				
					FVector ChunkPos = OffsetHexToWorld(NewChunkPos);

					AHexGridISMActor* HexActor = Cast<AHexGridISMActor>(GetWorld()->SpawnActor(CacheOpt->HexGridActorClass_, &ChunkPos));
					if (!ensure(HexActor != nullptr))
						return;
					
					HexActor->SetChunkCoord(ChunkCoord);
					
					ChunkIndex = ChunksList_.Emplace(ChunkCoord, HexActor);
					uint32 InKey = FChunkData::CalcKey(ChunkCoord);
					ChunkIndexes_.Add(InKey, ChunkIndex);
					
					UE_LOG(HexGridLog, Verbose, TEXT("New ChunkCoord: %s; ChunkIndex: %d; InKey: %u"), 
						*ChunkCoord.ToString(), ChunkIndex, InKey);
				}
				else
				{
					UE_LOG(HexGridLog, Verbose, TEXT("Old ChunkCoord: %s"), *ChunkCoord.ToString());
				}
			}
	}
}

int32 UHexagonChunkGrid::FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const
{
	uint32 InKey = FChunkData::CalcKey(InChunkCoord);
	for(int32 i = ChunkIndexes_.First(InKey); ChunkIndexes_.IsValid(i); i = ChunkIndexes_.Next(i))
	{
		if( ChunksList_[i].Key == InKey )
		{
			return i;
		}
	}
	return INDEX_NONE;
}

void UHexagonChunkGrid::SelectCell(const FVector& InPosition)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	TOptional<HexMath::FAxialCoord> AxialCell = WorldToAxialCellCoord(InPosition);
	if (!AxialCell.IsSet())
		return;

	HexMath::FOffsetCoord  OffsetCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(AxialCell.GetValue());
	
	SelectCell(OffsetCell, CacheOpt->NumColsRows);
	
	for (uint8 i = 0; i < HexMath::HexMathAxial::AxialNeighbourCount; ++i)
	{
		HexMath::FAxialCoord NAxialCell = AxialCell.GetValue() + HexMath::HexMathAxial::AxialNeighboursShifts[i];
		HexMath::FOffsetCoord NCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(NAxialCell);
		
		SelectCell(NCell, CacheOpt->NumColsRows);
	}
}

void UHexagonChunkGrid::SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, uint32 InNumColsRows)
{
	HexMath::FOffsetCoord ChunkCoord = HexMath::HexMathOffset::OffsetCellToChunk(InOffsetCoord, InNumColsRows, InNumColsRows);
	
	UE_LOG(HexGridLog, Warning, TEXT("ChunkCoord: %s"), *ChunkCoord.ToString());
	
	int32 ChunkIndex = FindChunkIndex(ChunkCoord);
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->SelectCell(InOffsetCoord);
	}
}

TOptional<HexMath::FOffsetCoord> UHexagonChunkGrid::WorldToChunkCoord(const FVector& InWorldPoint)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return {};
	
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CacheOpt->HexSize;
	uint32 NumColsRows = CacheOpt->NumColsRows;
	
	FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(InWorldPoint, HexSize);
	
	FOffsetCoord OCoord = HexMath::HexMathAxial::WorldToOffset<HEX_LAYOUT>(Coord, HexSize);
	
	FOffsetCoord ChunkCoord = OffsetCellToChunk(OCoord, NumColsRows, NumColsRows);
	
	return ChunkCoord;
}

TOptional<HexMath::FAxialCoord> UHexagonChunkGrid::WorldToAxialCellCoord(const FVector& InWorldPoint)
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

