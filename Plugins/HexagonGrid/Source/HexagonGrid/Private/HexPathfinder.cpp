#include "HexPathfainer.h"

DEFINE_LOG_CATEGORY_STATIC(HexPathfinderLog, Log, Log);

FString HexMath::FWaveElem::ToString() const
{
	return FString::Printf(TEXT("%s[%d] D: %u"), *Coord.ToString(), Rotation, Distance);
}

void HexMath::FindPathZone(const HexMath::FAxialCoord& InStart, int8 InStartRotation, uint32 InMaxWave,
	TSet<FWaveElem>& OutPath, bool InLog /*= false*/)
{
	OutPath.Reset();
	
	if (InLog)
	{
		UE_LOG(HexPathfinderLog, Log, TEXT("FindPathZone. InStart: %s; InStartRotation: %d; InMaxWave: %u"), 
			*InStart.ToString(), InStartRotation, InMaxWave);
	}
	
	auto IsWalkable = [](const HexMath::FAxialCoord& Coord) { return true; };// !Obstacles_.Contains(Coord); };
	
	auto MinHeapPred = [](const FWaveElem& A, const FWaveElem& B)
	{
		return A.Distance < B.Distance;
	};
	
	TArray<FWaveElem> Open;
	Open.Reserve(256);
	Open.HeapPush(FWaveElem{ InStart, InStartRotation, 0 }, MinHeapPred);
	
	uint32 Step = 0;
	while (Open.Num() > 0)
	{
		Step++;
    	
		FWaveElem Current;
		Open.HeapPop(Current, MinHeapPred, EAllowShrinking::No);
    	
		if (InLog)
		{
			UE_LOG(HexPathfinderLog, Log, TEXT("\t %u: OpenNum: %d; Current: %s"), Step, Open.Num(), *Current.ToString());
		}
		
		for (int32 i = 0; i < HexMath::HexMathAxial::AxialNeighbourCount; ++i)
		{
			const HexMath::FAxialCoord NeighbourCoord = Current.Coord + HexMath::HexMathAxial::AxialNeighboursShifts[i];
			const int8 AngleToNeighbour = HexMath::HexMathAxial::AxialNeighboursRotation[i];
			
			int RD1 = FMath::Abs(Current.Rotation - AngleToNeighbour);
			constexpr int8 LeftLimit = -3;
			constexpr int8 RightLimit = 3;
			int RD2 = 0; 
			if (AngleToNeighbour > Current.Rotation)
				RD2 = Current.Rotation - LeftLimit + RightLimit - AngleToNeighbour;
			else if (AngleToNeighbour < Current.Rotation)
				RD2 = AngleToNeighbour - LeftLimit + RightLimit - Current.Rotation;
			
			int8 RotationDiff = FMath::Min(RD1, RD2);			

			// Подставь свои проверки:
			// if (!IsValidCoord(N)) continue;
			if (!IsWalkable(NeighbourCoord)) continue;

			// Стоимость шага: 1 (или возьми из тайла)
			const uint32 StepCost = 1 + RotationDiff;// GetTraversalCost(Current.Coord, Neighbour);
			const uint32 TentativeDistance = Current.Distance + StepCost;
			
			if (TentativeDistance > InMaxWave)
			{
				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Too far! NeighbourCoord: %s; TentativeDistance: %u"), 
					   *NeighbourCoord.ToString(), TentativeDistance);
				}
				continue;
			}		

			FWaveElem& Neighbour = OutPath.FindOrAdd(FWaveElem{ NeighbourCoord, AngleToNeighbour, TNumericLimits<uint32>::Max() });

			if (TentativeDistance < Neighbour.Distance)
			{
				Neighbour.Distance = TentativeDistance;
				bool bMaxDist = Neighbour.Distance == InMaxWave;

				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Added Neighbour: %s; bMaxDist: %d"), *Neighbour.ToString(), bMaxDist);
				}
            	
				if (!bMaxDist)
				{
					Open.HeapPush(Neighbour, MinHeapPred);
				}
			}
			else if (InLog)
			{
				UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Missed Neighbour: %s"), *Neighbour.ToString());
			}
		}
	}

}