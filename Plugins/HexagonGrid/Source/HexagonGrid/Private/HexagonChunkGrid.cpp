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
	Cache.SelectRadius = Settings->SelectRadius;
	
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
	TOptional<FHexGridActorCDODataCache> CacheOpt = GetHexGridActorCDODataCache();
	if (!ensure(CacheOpt.IsSet()))
		return;
	
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
			HexMath::FOffsetCoord LastCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(SelectedCells_.Last());
			SelectCell(LastCell, CacheOpt->NumColsRows, false);
			
			SelectedCells_.Last() = AxialCell;
		}
		
		HexMath::FOffsetCoord NCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(AxialCell);
		SelectCell(NCell, CacheOpt->NumColsRows, true);
	}
	else
	{
		HexMath::FOffsetCoord LastCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(SelectedCells_[Index]);
		SelectCell(LastCell, CacheOpt->NumColsRows, false);
		SelectedCells_.RemoveAt(Index);
	}	
}

void UHexagonChunkGrid::SelectCell(const HexMath::FOffsetCoord& InOffsetCoord, uint32 InNumColsRows, bool InSelected)
{
	HexMath::FOffsetCoord ChunkCoord = HexMath::HexMathOffset::OffsetCellToChunk(InOffsetCoord, InNumColsRows, InNumColsRows);
	
	UE_LOG(HexGridLog, Warning, TEXT("ChunkCoord: %s"), *ChunkCoord.ToString());
	
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
		[this, InCellType, NumColsRows = CacheOpt->NumColsRows] 
			(const HexMath::FAxialCoord& InCell)
	{
		HexMath::FOffsetCoord NCell = HexMath::HexMathAxial::AxialToOffset<HEX_LAYOUT>(InCell);
		
		SetCellType(NCell, NumColsRows, InCellType);
	});
}

void UHexagonChunkGrid::SetCellType(const HexMath::FOffsetCoord& InOffsetCoord, uint32 InNumColsRows, ECellType InCellType)
{
	HexMath::FOffsetCoord ChunkCoord = HexMath::HexMathOffset::OffsetCellToChunk(InOffsetCoord, InNumColsRows, InNumColsRows);
	
	UE_LOG(HexGridLog, Warning, TEXT("ChunkCoord: %s"), *ChunkCoord.ToString());
	
	int32 ChunkIndex = FindChunkIndex(ChunkCoord);
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->SetCellType(InOffsetCoord, InCellType);
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

UHexagonChunkGrid::FNodeRef UHexagonChunkGrid::GetNeighbour(const FNodeRef& NodeRef, const int32 NeighbourIndex) const
{
	if (NeighbourIndex >= HexMath::HexMathAxial::AxialNeighbourCount)
	{
		return FNodeRef();
	}
	
	FNodeRef N = NodeRef + HexMath::HexMathAxial::AxialNeighboursShifts[NeighbourIndex];
	return N;
}

void UHexagonChunkGrid::FindPath(const HexMath::FAxialCoord& Start, const HexMath::FAxialCoord& End,
	TArray<HexMath::FAxialCoord>& OutPath)
{
	OutPath.Reset();

    // Быстрые случаи
    if (Start == End)
    {
        OutPath.Add(Start);
        return;
    }

    // Эти функции/проверки подставь под свою сетку:
    // if (!IsValidCoord(Start) || !IsValidCoord(End)) return;
    // if (!IsWalkable(Start) || !IsWalkable(End)) return;

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
        return A.F > B.F; // меньше F = выше приоритет
    };

    TArray<FOpenNode> Open;
    Open.Reserve(256);

    TMap<HexMath::FAxialCoord, int32> GScore;
    GScore.Reserve(256);
    GScore.Add(Start, 0);

    TMap<HexMath::FAxialCoord, HexMath::FAxialCoord> CameFrom;
    CameFrom.Reserve(256);

    // closed можно не хранить отдельно, но удобно
    TSet<HexMath::FAxialCoord> Closed;
    Closed.Reserve(256);

    {
        const int32 H = AxialDistance(Start, End);
        Open.HeapPush(FOpenNode{ Start, /*F*/ H, /*G*/ 0 }, MinHeapPred);
    }

    while (Open.Num() > 0)
    {
        FOpenNode Current;
        Open.HeapPop(Current, MinHeapPred, EAllowShrinking::No);

        // Отбрасываем устаревшие записи (из-за отсутствия decrease-key)
        const int32* BestG = GScore.Find(Current.Coord);
        if (!BestG || Current.G != *BestG)
        {
            continue;
        }

        if (Current.Coord == End)
        {
            // Восстановление пути
            TArray<HexMath::FAxialCoord> ReversePath;
            ReversePath.Reserve(64);

            HexMath::FAxialCoord C = End;
            ReversePath.Add(C);

            while (!(C == Start))
            {
                HexMath::FAxialCoord* Parent = CameFrom.Find(C);
                if (!Parent)
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
            const HexMath::FAxialCoord N = Current.Coord + HexMath::HexMathAxial::AxialNeighboursShifts[i];

            // Подставь свои проверки:
            // if (!IsValidCoord(N)) continue;
            // if (!IsWalkable(N)) continue;

            // Стоимость шага: 1 (или возьми из тайла)
            const int32 StepCost = GetTraversalCost(Current.Coord, N);
            const int32 TentativeG = Current.G + StepCost;

            int32* OldG = GScore.Find(N);
            if (!OldG || TentativeG < *OldG)
            {
                CameFrom.Add(N, Current.Coord);
                GScore.Add(N, TentativeG);

                const int32 H = AxialDistance(N, End);
                const int32 F = TentativeG + H;

                Open.HeapPush(FOpenNode{ N, F, TentativeG }, MinHeapPred);
            }
        }
    }

    // пути нет
    OutPath.Reset();
	//FGraphAStar<UHexagonChunkGrid> Pathfinder(*this);
	
	//TArray<HexMath::FAxialCoord> OutPathIndices;
	//const EGraphAStarResult Result = Pathfinder.FindPath(Start, End, *this, OutPathIndices);
}