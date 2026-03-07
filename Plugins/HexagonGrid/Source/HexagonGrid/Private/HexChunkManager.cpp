#include "HexChunkManager.h"
#include "HexagonChunkGrid.h"
#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"
#include "HexGridUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(HexChunkManagerLog, Log, Log);

UHexagonChunkGrid* UFHexChunkManager::GetOwner() const
{
	return Cast<UHexagonChunkGrid>(GetOuter());
}

void UFHexChunkManager::CreateNewChunks(const FVector& InNewPosition)
{
	TOptional<HexMath::FOffsetCoord> ObserverChunkCoordOp = HexGridUtilities::WorldToChunkCoord(InNewPosition);
	if (!ObserverChunkCoordOp.IsSet())
		return;
	
	CreateNewChunks(ObserverChunkCoordOp.GetValue());
}
void UFHexChunkManager::CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition)
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
					
					UE_LOG(HexChunkManagerLog, Verbose, TEXT("New ChunkCoord: %s; ChunkIndex: %d; InKey: %u"), 
						*ChunkCoord.ToString(), ChunkIndex, InKey);
				}
				else
				{
					UE_LOG(HexChunkManagerLog, Verbose, TEXT("Old ChunkCoord: %s"), *ChunkCoord.ToString());
				}
			}
	}
}

int32 UFHexChunkManager::FindChunkIndex(const HexMath::FOffsetCoord& InChunkCoord) const
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

void UFHexChunkManager::SetCellType(const HexMath::FAxialCoord& InAxialCoord, ECellType InCellType, float InLevel)
{
	HexMath::FOffsetCoord NCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	SetCellType(NCell, InCellType, InLevel);
}

void UFHexChunkManager::SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType, float InLevel)
{
	uint32 ColRowCount = HexGridUtilities::GetColRowCountInChunk();
	HexMath::FOffsetCoord ChunkCoord = HexMath::HexMathOffset::OffsetCellToChunk(InOffsetCoord, ColRowCount, ColRowCount);
	
	UE_LOG(HexChunkManagerLog, Verbose, TEXT("ChunkCoord: %s"), *ChunkCoord.ToString());
	
	int32 ChunkIndex = FindChunkIndex(ChunkCoord);
	
	if (ChunkIndex == INDEX_NONE)
	{
		CreateNewChunks(ChunkCoord);
		ChunkIndex = FindChunkIndex(ChunkCoord);
	}
	
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->SetCellType(InOffsetCoord, InCellType, InLevel);
	}
}

void UFHexChunkManager::SetCellType(const FVector& InPosition, ECellType InCellType)
{
	TOptional<HexGridUtilities::FHexGridActorCDODataCache> CacheOpt = HexGridUtilities::GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	TOptional<HexMath::FAxialCoord> AxialCell = HexGridUtilities::WorldToAxialCellCoord(InPosition);
	if (!AxialCell.IsSet())
		return;
	
	HexMath::HexMathAxial::IterateAxialNeighbours(AxialCell.GetValue(), CacheOpt->SelectRadius, 
		[this, InCellType] (const HexMath::FAxialCoord& InCell)
	{
		SetCellType(InCell, InCellType, 1.f);
			
		if (InCellType != ECellType::Opened)
		{
			Obstacles_.Add(InCell);
		}
		else
		{
			Obstacles_.Remove(InCell);
		}
	});
}