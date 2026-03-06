// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonChunkGrid.h"
#include "HexMath.h"
#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridLog, Log, Log);

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
	Cache.SelectRadius = Settings->SelectRadius;
	Cache.PathfinderLog = Settings->PathfinderLog;

	return Cache;
}

uint32 UHexagonChunkGrid::GetColRowCountInChunk()
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return 3;
	return CacheOpt->NumColsRows;
}

void UHexagonChunkGrid::CreateNewChunks(const FVector& InNewPosition)
{
	TOptional<HexMath::FOffsetCoord> ObserverChunkCoordOp = WorldToChunkCoord(InNewPosition);
	if (!ObserverChunkCoordOp.IsSet())
		return;
	
	CreateNewChunks(ObserverChunkCoordOp.GetValue());
}
void UHexagonChunkGrid::CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition)
{
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
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

TOptional<HexMath::FOffsetCoord> UHexagonChunkGrid::WorldToChunkCoord(const FVector& InWorldPoint)
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

void UHexagonChunkGrid::SetCellType(const FVector& InPosition, ECellType InCellType)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	TOptional<HexMath::FAxialCoord> AxialCell = WorldToAxialCellCoord(InPosition);
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

void UHexagonChunkGrid::FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath, bool InDrawHexes)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;

	DropPathSelections();
	
	HexMath::FindPath(InStart, InStartRotation, InEnd, InEndRotation, InMoveParams, OutPath, CacheOpt->PathfinderLog);
	
	if (InDrawHexes)
	{
		PFCells_ = OutPath;
	
		for (HexMath::FPathNode PFCell : PFCells_)
			SetCellType(PFCell.Coord, ECellType::Selected, 1);
	}
}

void UHexagonChunkGrid::DropPathSelections()
{
	for (HexMath::FPathNode PFCell : PFCells_)
		SetCellType(PFCell.Coord, ECellType::Selected, 0);
}

void UHexagonChunkGrid::SetCellType(const HexMath::FAxialCoord& InAxialCoord, ECellType InCellType, float InLevel)
{
	HexMath::FOffsetCoord NCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	SetCellType(NCell, InCellType, InLevel);
}

void UHexagonChunkGrid::SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, ECellType InCellType, float InLevel)
{
	uint32 ColRowCount = GetColRowCountInChunk();
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
		ChunksList_[ChunkIndex].ChunkActor->SetCellType(InOffsetCoord, InCellType, InLevel);
	}
}

void UHexagonChunkGrid::CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation, const FHullHexFootprint& InHull)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	ReleaseCells(InId);
	
	TArray<HexMath::FAxialCoord>& Cells = CapturedCells_.FindOrAdd(InId);

	HexMath::FAxialCoord HexCenterCell = InCenterCell;
	
	HexMath::CaptureCells(HexCenterCell, InRotation, InHull, Cells, CacheOpt->PathfinderLog);
	
	for (HexMath::FAxialCoord Cell : Cells)
	{
		SetCellType(Cell, ECellType::Captured, 1);
	}
}

void UHexagonChunkGrid::ReleaseCells(uint32 InId)
{
	ClearCells(InId, ECellType::Captured);
}

void UHexagonChunkGrid::SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation,
	const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	RemoveInfluence(InId);
	
	TArray<HexMath::FAxialCoord>& Cells = InfluencedCells_.FindOrAdd(InId);

	HexMath::FAxialCoord HexCenterCell = InHexCell;
	
	TSet<HexMath::FPathNode> Wave;
	HexMath::FindPathZone(HexCenterCell, InRotation, InMoveParams, Wave, CacheOpt->PathfinderLog);
	
	for (const HexMath::FPathNode& WaveElem : Wave)
	{
		Cells.Add(WaveElem.Coord);
		float Level = 1 - static_cast<float>(WaveElem.Distance) / (InMoveParams.MaxDistance + InMoveParams.MoveCost);
		SetCellType(WaveElem.Coord, ECellType::MoveProjection, Level);
		if (OutPath)
			OutPath->Add(WaveElem);
	}
}

void UHexagonChunkGrid::RemoveInfluence(uint32 InId)
{
	ClearCells(InId, ECellType::MoveProjection);
}

void UHexagonChunkGrid::ClearCells(uint32 InId, ECellType InCellType)
{
	TArray<HexMath::FAxialCoord>* Cells = nullptr;
	if (InCellType == ECellType::Captured)
		Cells = CapturedCells_.Find(InId);
	else if (InCellType == ECellType::MoveProjection)
		Cells = InfluencedCells_.Find(InId);
	else
	{
		UE_LOG(HexGridLog, Error, TEXT("ClearCells. InCellType:%s"), *StaticEnum<ECellType>()->GetValueAsString(InCellType));
		return;
	}
	
	if (Cells != nullptr)
	{
		for (HexMath::FAxialCoord Cell : *Cells)
		{
			SetCellType(Cell, InCellType, 0);
		}
		Cells->Reset();
	}
}