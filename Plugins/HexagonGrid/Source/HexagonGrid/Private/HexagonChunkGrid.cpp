// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonChunkGrid.h"

#include "HexagonGridSettings.h"
#include "HexMath.h"
#include "HexGridISMActor.h"
#include "HexGridUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridLog, Log, Log);


void UHexagonChunkGrid::OnChangeObserverPosition(const FVector& InNewPosition)
{
	CreateNewChunks(InNewPosition);	
}


void UHexagonChunkGrid::SetCellDrawing(const HexMath::FAxialCoord& InAxialCoord, const FHexCellDrawInfo& InCellInfo)
{
	OnCellChange(InAxialCoord, InCellInfo);
}

void UHexagonChunkGrid::CreateNewChunks(const FVector& InNewPosition)
{
	TOptional<HexMath::FOffsetCoord> ObserverChunkCoordOp = HexGridUtilities::WorldToChunkCoord(InNewPosition);
	if (!ObserverChunkCoordOp.IsSet())
		return;
	
	CreateNewChunks(ObserverChunkCoordOp.GetValue());
}

void UHexagonChunkGrid::CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition)
{
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	TOptional<HexGridUtilities::FHexGridActorCDODataCache> CacheOpt = HexGridUtilities::GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	float HexSize = CacheOpt->HexSize;
	uint32 NumColsRows = CacheOpt->NumColsRows;

	FOffsetRealCoord ChunkPitch = GetChunkPitchNoGap<HEX_LAYOUT>(NumColsRows, NumColsRows, HexSize);
	
	if (CurrentChunkCoord_ != InNewPosition)
	{
		CurrentChunkCoord_ = InNewPosition;
		
		int32 iT = CacheOpt->BuildChunkAround;
		
		for (HexInt Idx = CurrentChunkCoord_.Row - iT; Idx <= CurrentChunkCoord_.Row + iT; ++Idx)
			for (HexInt Jdx = CurrentChunkCoord_.Col - iT; Jdx <= CurrentChunkCoord_.Col + iT; ++Jdx)
			{
				FOffsetCoord ChunkCoord(Jdx, Idx);
				int32 ChunkIndex = FindChunkIndex(ChunkCoord);
				bool bAlreadyPresent = ChunkIndex != INDEX_NONE;
				
				if (!bAlreadyPresent)
				{
					FOffsetRealCoord NewChunkPos(ChunkCoord.Col * ChunkPitch.Right,  ChunkCoord.Row * ChunkPitch.Up);
				
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

void UHexagonChunkGrid::OnCellChange(const HexMath::FAxialCoord& InAxialCoord, const FHexCellDrawInfo& InCellInfo)
{
	HexMath::FOffsetCoord InOffsetCoord = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	
	uint32 ColRowCount = HexGridUtilities::GetColRowCountInChunk();
	HexMath::FOffsetCoord ChunkCoord = HexMath::HexMathOffset::OffsetCellToChunk(InOffsetCoord, ColRowCount, ColRowCount);
	
	UE_LOG(HexGridLog, Verbose, TEXT("ChunkCoord: %s"), *ChunkCoord.ToString());
	
	int32 ChunkIndex = FindChunkIndex(ChunkCoord);
	
	if (ChunkIndex == INDEX_NONE)
	{
		CreateNewChunks(ChunkCoord);
		ChunkIndex = FindChunkIndex(ChunkCoord);
	}
	
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->OnCellChange(InOffsetCoord, InCellInfo);
	}
}