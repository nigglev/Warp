// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonChunkGrid.h"
#include "HexMath.h"
#include "HexChunkManager.h"
#include "HexGridUtilities.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridLog, Log, Log);

UHexagonChunkGrid::UHexagonChunkGrid()
{
	ChunkManager_ = CreateDefaultSubobject<UFHexChunkManager>(TEXT("ChunkManager"));
}

void UHexagonChunkGrid::OnChangeObserverPosition(const FVector& InNewPosition)
{
	ChunkManager_->CreateNewChunks(InNewPosition);	
}

void UHexagonChunkGrid::CreateNewChunks(const HexMath::FOffsetCoord& InNewPosition)
{
	ChunkManager_->CreateNewChunks(InNewPosition);
}

void UHexagonChunkGrid::FindPath(const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd, const TOptional<int8>& InEndRotation,
		const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath, bool InDrawHexes)
{
	TOptional<HexGridUtilities::FHexGridActorCDODataCache> CacheOpt = HexGridUtilities::GetHexGridActorCDODataCache();
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


void UHexagonChunkGrid::CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation, const FHullHexFootprint& InHull)
{
	TOptional<HexGridUtilities::FHexGridActorCDODataCache> CacheOpt = HexGridUtilities::GetHexGridActorCDODataCache();
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
	TOptional<HexGridUtilities::FHexGridActorCDODataCache> CacheOpt = HexGridUtilities::GetHexGridActorCDODataCache();
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

void UHexagonChunkGrid::SetCellType(const HexMath::FAxialCoord& InAxialCoord, ECellType InCellType, float InLevel)
{
	FCellLayers* Cell = SelectStatus_.Find(InAxialCoord);
	if (Cell == nullptr && InLevel == 0)
		return;
	
	if (Cell == nullptr)
	{
		Cell = &SelectStatus_.Emplace(InAxialCoord);
	}
	
	Cell->SetCellType(InCellType, InLevel);
	if (Cell->GetCellType() == ECellType::Opened)
	{
		SelectStatus_.Remove(InAxialCoord);
	}
	
	ChunkManager_->OnCellChange(InAxialCoord, *Cell);
}

void UHexagonChunkGrid::SetCellType(const FVector& InPosition, ECellType InCellType)
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