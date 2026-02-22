// Fill out your copyright notice in the Description page of Project Settings.

#include "HexagonChunkGrid.h"
#include "HexMath.h"
#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"
#include "HexPathfainer.h"

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
	Cache.PathfinderRotationCost = Settings->PathfinderRotationCost;
	
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
		
		int32 iT = CacheOpt->BuildChunkAround;
		
		for (HexInt Idx = CurrentChunkCoord_.Up - iT; Idx <= CurrentChunkCoord_.Up + iT; ++Idx)
			for (HexInt Jdx = CurrentChunkCoord_.Right - iT; Jdx <= CurrentChunkCoord_.Right + iT; ++Jdx)
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
	TOptional<HexMath::FAxialCoord> AxialCellOpt = WorldToAxialCellCoord(InPosition);
	if (!AxialCellOpt.IsSet())
		return;
	
	HexMath::FAxialCoord AxialCell = AxialCellOpt.GetValue();
	
	int32 Index = SelectedCells_.Find(AxialCell);
	if (Index == INDEX_NONE)
	{
		if (SelectedCells_.Num() < 2)
		{
			SelectedCells_.Add(AxialCell);
		}
		else
		{
			SelectCell(SelectedCells_.Last(), false);
			
			SelectedCells_.Last() = AxialCell;
		}
		
		SelectCell(AxialCell, true);
	}
	else
	{
		SelectCell(SelectedCells_[Index], false);
		SelectedCells_.RemoveAt(Index);
	}
	
	if (SelectedCells_.Num() == 2)
	{
		FindPath(SelectedCells_[0], SelectedCells_.Last(), PFCells_);
	}
}

void UHexagonChunkGrid::SelectCell(const HexMath::FAxialCoord& InAxialCoord, bool InSelected)
{
	HexMath::FOffsetCoord NCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	SelectCell(NCell, InSelected);
}

void UHexagonChunkGrid::SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, bool InSelected)
{
	uint32 ColRowCount = GetColRowCountInChunk();
	HexMath::FOffsetCoord ChunkCoord = HexMath::HexMathOffset::OffsetCellToChunk(InOffsetCoord, ColRowCount, ColRowCount);
	
	UE_LOG(HexGridLog, Verbose, TEXT("ChunkCoord: %s"), *ChunkCoord.ToString());
	
	int32 ChunkIndex = FindChunkIndex(ChunkCoord);
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->SelectCell(InOffsetCoord, InSelected);
	}
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
		SetCellType(InCell, InCellType);
			
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

void UHexagonChunkGrid::SelectInfluence(uint32 InId, const HexMath::FAxialCoord& InHexCell, int8 InRotation, uint32 InHexDistance,
	TArray<HexMath::FAxialCoord>* OutPath)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	RemoveInfluence(InId);
	
	TArray<HexMath::FAxialCoord>& Cells = InfluencedCells_.FindOrAdd(InId);

	HexMath::FAxialCoord HexCenterCell = InHexCell;
	
	TSet<HexMath::FWaveElem> Wave;
	FVector2D Costs(1, CacheOpt->PathfinderRotationCost);
	HexMath::FindPathZone(HexCenterCell, InRotation, InHexDistance, Wave, Costs, CacheOpt->PathfinderLog);
	
	for (const HexMath::FWaveElem& WaveElem : Wave)
	{
		Cells.Add(WaveElem.Coord);
		float Level = static_cast<float>(WaveElem.Distance) / (InHexDistance + 1);
		SetCellType(WaveElem.Coord, ECellType::Captured, Level);
		if (OutPath)
			OutPath->Add(WaveElem.Coord);
	}
}

void UHexagonChunkGrid::RemoveInfluence(uint32 InId)
{
	TArray<HexMath::FAxialCoord>* Cells = InfluencedCells_.Find(InId);
	if (Cells)
	{
		for (HexMath::FAxialCoord Cell : *Cells)
		{
			SetCellType(Cell, ECellType::Opened);
		}
		Cells->Reset();
	}
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
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->SetCellType(InOffsetCoord, InCellType, InLevel);
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

void UHexagonChunkGrid::SelectedFindPath(const HexMath::FAxialCoord& InStart, const HexMath::FAxialCoord& InEnd,
	TArray<HexMath::FAxialCoord>& OutPath)
{
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
	for (HexMath::FAxialCoord PFCell : PFCells_)
		SelectCell(PFCell, false);
	
	FindPath(InStart, InEnd, OutPath, CacheOpt->PathfinderLog);
	
	PFCells_ = OutPath;
	
	for (HexMath::FAxialCoord PFCell : PFCells_)
		SelectCell(PFCell, true);
}

void UHexagonChunkGrid::FindPath(const HexMath::FAxialCoord& InStart, const HexMath::FAxialCoord& InEnd,
	TArray<HexMath::FAxialCoord>& OutPath, bool InLog /*= false*/) const
{
	OutPath.Reset();
	
	if (InLog)
	{
		UE_LOG(HexGridLog, Log, TEXT("FindPath. InStart: %s; InEnd: %s"), *InStart.ToString(), *InEnd.ToString());
	}

    // Быстрые случаи
    if (InStart == InEnd)
    {
        OutPath.Add(InStart);
        return;
    }
	
	auto IsWalkable = [this](const HexMath::FAxialCoord& Coord) { return !Obstacles_.Contains(Coord); };

    // Эти функции/проверки подставь под свою сетку:
    // if (!IsValidCoord(Start) || !IsValidCoord(End)) return;
    if (!IsWalkable(InStart) || !IsWalkable(InStart)) return;

    struct FOpenNode
    {
        HexMath::FAxialCoord Coord;
        int32 F = 0;   // g + h
        int32 G = 0;   // стоимость от Start
    };

    // Min-heap по F: меньший F должен быть "наверху".
    // В UE heap-алгоритмах часто нужно инвертировать сравнение для min-heap.
    auto MinHeapPred = [](const FOpenNode& A, const FOpenNode& B)
    {
        return A.F < B.F; // меньше F = выше приоритет
    };

    TArray<FOpenNode> Open;
    Open.Reserve(256);

    TMap<HexMath::FAxialCoord, int32> GScore;
    GScore.Reserve(256);
    GScore.Add(InStart, 0);

    TMap<HexMath::FAxialCoord, HexMath::FAxialCoord> CameFrom;
    CameFrom.Reserve(256);

    // closed можно не хранить отдельно, но удобно
    TSet<HexMath::FAxialCoord> Closed;
    Closed.Reserve(256);

    {
        const int32 H = AxialDistance(InStart, InEnd);
        Open.HeapPush(FOpenNode{ InStart, /*F*/ H, /*G*/ 0 }, MinHeapPred);
    }

	uint32 Step = 0;
    while (Open.Num() > 0)
    {
    	Step++;
    	
        FOpenNode Current;
        Open.HeapPop(Current, MinHeapPred, EAllowShrinking::No);
    	
    	if (InLog)
    	{
    		UE_LOG(HexGridLog, Log, TEXT("\t %u: OpenNim: %d; Coord: %s; F: %d G: %d"), Step, Open.Num(), *Current.Coord.ToString(), Current.F, Current.G);
    	}

        if (Current.Coord == InEnd)
        {
            // Восстановление пути
            TArray<HexMath::FAxialCoord> ReversePath;
            ReversePath.Reserve(64);

            HexMath::FAxialCoord C = InEnd;
            ReversePath.Add(C);

            while (C != InStart)
            {
                HexMath::FAxialCoord* Parent = CameFrom.Find(C);
                if (!ensure(Parent))
                {
                    OutPath.Reset();
                    return;
                }
                C = *Parent;
                ReversePath.Add(C);
            }

            Algo::Reverse(ReversePath);
            OutPath = MoveTemp(ReversePath);
            return;
        }

        if (Closed.Contains(Current.Coord))
        {
            continue;
        }
        Closed.Add(Current.Coord);

        for (int32 i = 0; i < HexMath::HexMathAxial::AxialNeighbourCount; ++i)
        {
            const HexMath::FAxialCoord Neighbour = Current.Coord + HexMath::HexMathAxial::AxialNeighboursShifts[i];

            // Подставь свои проверки:
            // if (!IsValidCoord(N)) continue;
            if (!IsWalkable(Neighbour)) continue;

            // Стоимость шага: 1 (или возьми из тайла)
            const int32 StepCost = 1;// GetTraversalCost(Current.Coord, Neighbour);
            const int32 TentativeG = Current.G + StepCost;

            int32* OldG = GScore.Find(Neighbour);
        	int32 OldGVal = OldG ? *OldG : -1;

            if (OldG == nullptr || TentativeG < *OldG)
            {
                CameFrom.Add(Neighbour, Current.Coord);
                GScore.Add(Neighbour, TentativeG);

                const int32 H = AxialDistance(Neighbour, InEnd);
                const int32 F = TentativeG + H;

            	if (InLog)
            	{
            		UE_LOG(HexGridLog, Log, TEXT("\t\t Added Neighbour: %s; OldG: %d; TentativeG: %d; F: %d"), 
					   *Neighbour.ToString(), OldGVal, TentativeG, F);
            	}
            	
                Open.HeapPush(FOpenNode{ Neighbour, F, TentativeG }, MinHeapPred);
            }
        	else if (InLog)
        	{
        		UE_LOG(HexGridLog, Log, TEXT("\t\t Missed Neighbour: %s; OldG: %d; TentativeG: %d"), 
					*Neighbour.ToString(), OldGVal, TentativeG);
        	}
        }
    }

    // пути нет
    OutPath.Reset();
	//FGraphAStar<UHexagonChunkGrid> Pathfinder(*this);
	
	//TArray<HexMath::FAxialCoord> OutPathIndices;
	//const EGraphAStarResult Result = Pathfinder.FindPath(Start, End, *this, OutPathIndices);
}
