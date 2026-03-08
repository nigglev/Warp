// Fill out your copyright notice in the Description page of Project Settings.


#include "HexMapWS.h"

#include "HexCellDrawInfo.h"
#include "HexGridWorldSubsystem.h"
#include "MGLogs.h"
#include "Algo/MaxElement.h"
#include "HexagonGrid/Private/HexagonChunkGrid.h"
#include "Warp/ContentManagement/GameSettings.h"

DEFINE_LOG_CATEGORY_STATIC(AHexMapWSLog, Log, All);

UHexMapWS* UHexMapWS::Get(const UObject* InWorldContextObject)
{
	return InWorldContextObject->GetWorld()->GetSubsystem<UHexMapWS>();
}

void UHexMapWS::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	HexGridWS_ = Collection.InitializeDependency<UHexGridWorldSubsystem>();
}

void UHexMapWS::Deinitialize()
{
	HexGridWS_ = nullptr;
	
	Super::Deinitialize();
}

void UHexMapWS::OnChangeObserverPosition(const FVector& InNewPosition)
{
	if (HexGridWS_)
		HexGridWS_->OnChangeObserverPosition(InNewPosition);
}

void UHexMapWS::SetCellType(const FVector& InPosition, ECellType InCellType)
{
}

void UHexMapWS::CaptureCells(uint32 InId, const HexMath::FAxialCoord& InCenterCell, int8 InRotation,
	const FHullHexFootprint& InHull)
{
	RETURN_ON_FAIL(AHexMapWSLog, HexGridWS_);
	
	ECellType CellType = ECellType::SuccessCaptured;
	
	ClearCells(InId, CellType);
	
	UGameSettings* GameSettings = UGameSettings::Get();
	RETURN_ON_FAIL(AHexMapWSLog, GameSettings);	
	
	FIdCellCollection* Collection = GetIdCellCollection(FCollectionKey(InId, CellType), true);
	RETURN_ON_FAIL(AHexMapWSLog, Collection);
	
	HexMath::FAxialCoord HexCenterCell = InCenterCell;
	
	HexMath::CaptureCells(HexCenterCell, InRotation, InHull, Collection->Cells, GameSettings->PathfinderLog);
	
	for (HexMath::FAxialCoord CellCoord : Collection->Cells)
	{
		ChangeCell(CellCoord, CellType, FCellIdData(InId, 1));
	}
}

void UHexMapWS::ReleaseCells(uint32 InId)
{
	ClearCells(InId, ECellType::SuccessCaptured);
}

void UHexMapWS::SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation,
	const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>* OutPath)
{
	RETURN_ON_FAIL(AHexMapWSLog, HexGridWS_);
	
	ECellType CellType = ECellType::MoveProjection;
	
	ClearCells(InId, CellType);
	
	UGameSettings* GameSettings = UGameSettings::Get();
	RETURN_ON_FAIL(AHexMapWSLog, GameSettings);	
	
	FIdCellCollection* Collection = GetIdCellCollection(FCollectionKey(InId, CellType), true);
	RETURN_ON_FAIL(AHexMapWSLog, Collection);
	
	TSet<HexMath::FPathNode> Wave;
	HexMath::FindPathZone(InHexCell, InRotation, InMoveParams, Wave, GameSettings->PathfinderLog);
	
	for (const HexMath::FPathNode& WaveElem : Wave)
	{
		Collection->Cells.Add(WaveElem.Coord);
		float Level = 1 - static_cast<float>(WaveElem.Distance) / (InMoveParams.MaxDistance + InMoveParams.MoveCost);
		ChangeCell(WaveElem.Coord, CellType, FCellIdData(InId, Level));
	}
}

void UHexMapWS::RemoveInfluence(uint32 InId)
{
	ClearCells(InId, ECellType::MoveProjection);
}

void UHexMapWS::FindPath(uint32 InId, const HexMath::FAxialCoord& InStart, int8 InStartRotation, const HexMath::FAxialCoord& InEnd,
	const TOptional<int8>& InEndRotation, const FMoveParams& InMoveParams, TArray<HexMath::FPathNode>& OutPath,
	bool InDrawHexes)
{
	DropPathSelections(InId);
	
	ECellType CellType = ECellType::MovingPath;
	
	UGameSettings* GameSettings = UGameSettings::Get();
	RETURN_ON_FAIL(AHexMapWSLog, GameSettings);	
	
	HexMath::FindPath(InStart, InStartRotation, InEnd, InEndRotation, InMoveParams, OutPath, GameSettings->PathfinderLog);
	
	if (InDrawHexes)
	{
		FIdCellCollection* Collection = GetIdCellCollection(FCollectionKey(InId, CellType), true);
		RETURN_ON_FAIL(AHexMapWSLog, Collection);
		
		for (HexMath::FPathNode PFCell : OutPath)
		{
			Collection->Cells.Add(PFCell.Coord);
			ChangeCell(PFCell.Coord, CellType, FCellIdData(InId, 1));
		}
	}
}

void UHexMapWS::DropPathSelections(uint32 InId)
{
	ClearCells(InId, ECellType::MovingPath);
}

TOptional<HexMath::FAxialCoord> UHexMapWS::WorldToAxialCellCoord(const FVector& InWorldPoint)
{
	return UHexGridWorldSubsystem::WorldToAxialCellCoord(InWorldPoint);
}

TOptional<FVector> UHexMapWS::AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset)
{
	return UHexGridWorldSubsystem::AxialCellToWorldCoord(InAxialCoord, InZOffset);
}

UHexMapWS::FIdCellCollection* UHexMapWS::GetIdCellCollection(FCollectionKey InKey, bool InCreate)
{
	int32 Index = Algo::UpperBoundBy(IdCellCollections_, InKey, 
		[](const FIdCellCollection& InCollection) { return InCollection.Key; },
		TLess<FCollectionKey>());
	
	FIdCellCollection* Collection = nullptr;
	if (IdCellCollections_.IsValidIndex(Index) && IdCellCollections_[Index].Key == InKey)
		Collection = &IdCellCollections_[Index];
	else if (InCreate)
	{
		Collection = &IdCellCollections_.InsertDefaulted_GetRef(Index);
		Collection->Key = InKey;
	}
	
	return Collection;
}

void UHexMapWS::ChangeCell(const HexMath::FAxialCoord& InCellCoord, ECellType InCellType, FCellIdData InData)
{
	FCell* Cell = Cells_.Find(InCellCoord);
	if (Cell == nullptr && InData.Level == 0)
		return;
	
	if (Cell == nullptr && InData.Level > 0)
		Cell = &Cells_.Emplace(InCellCoord);
	
	TArray<FCellIdData>& CellData = Cell->Counters[static_cast<uint8>(InCellType)];
	FCellIdData* IdData = CellData.FindByPredicate([InId = InData.Id](const FCellIdData& InData) { return InData.Id == InId; });
	if (IdData == nullptr)
		CellData.Add(InData);
	else
		*IdData = InData;
	
	OnChangeCell(InCellCoord, *Cell);
	
	if (TryClearCell(*Cell))
		Cells_.Remove(InCellCoord);
}

bool UHexMapWS::TryClearCell(FCell& InCell)
{
	bool bEmpty = true;
	for (uint8 i = static_cast<uint8>(ECellType::MAX) - 1; i > 0; i--)
	{
		TArray<FCellIdData>& CellData = InCell.Counters[i];
		if (!CellData.IsEmpty())
		{
			const FCellIdData* Data = Algo::MaxElementBy(CellData, [](const FCellIdData& InData)
			{
				return InData.Level;
			});
			
			if (Data->Level == 0)
				CellData.Empty();
			else
				bEmpty = false;
		}
	}
	return bEmpty;
}

void UHexMapWS::OnChangeCell(const HexMath::FAxialCoord& InCellCoord, FCell& InCell)
{
	FHexCellDrawInfo CellInfo;
	for (uint8 i = static_cast<uint8>(ECellType::MAX) - 1; i > 0; i--)
	{
		TArray<FCellIdData>& CellData = InCell.Counters[i];
		if (!CellData.IsEmpty())
		{
			const FCellIdData* Data = Algo::MaxElementBy(CellData, [](const FCellIdData& InData)
			{
				return InData.Level;
			});
			
			if (Data->Level > 0)
			{
				CellInfo.Level = Data->Level;
				CellInfo.CellType = static_cast<ECellType>(i);
				break;
			}
		}
	}

	HexGridWS_->GetGrid()->SetCellDrawing(InCellCoord, CellInfo);
}

void UHexMapWS::ClearCells(uint32 InId, ECellType InCellType)
{
	int32 Index = Algo::BinarySearchBy(IdCellCollections_, FCollectionKey(InId, InCellType),
		[](const FIdCellCollection& InCollection) { return InCollection.Key; },
		TLess<FCollectionKey>());
	
	if (Index == INDEX_NONE)
		return;
	
	for (HexMath::FAxialCoord CellCoord : IdCellCollections_[Index].Cells)
	{
		ChangeCell(CellCoord, InCellType, FCellIdData(InId, 0));
	}
	
	IdCellCollections_.RemoveAt(Index);
}
