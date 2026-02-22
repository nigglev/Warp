#include "HexPathfainer.h"

DEFINE_LOG_CATEGORY_STATIC(HexPathfinderLog, Log, Log);

FString HexMath::FWaveElem::ToString() const
{
	return FString::Printf(TEXT("%s[%d] D: %.2f"), *Coord.ToString(), Rotation, Distance);
}

void HexMath::FindPathZone(const HexMath::FAxialCoord& InStart, int8 InStartRotation, float InMaxWave,
	TSet<FWaveElem>& OutPath, FVector2D InStepRotationPrice, bool InLog)
{
	OutPath.Reset();
	
	if (InLog)
	{
		UE_LOG(HexPathfinderLog, Log, TEXT("FindPathZone. InStart: %s; InStartRotation: %d; InMaxWave: %.2f; StepCost: %.2f; Rotation Cost: %.2f"), 
			*InStart.ToString(), InStartRotation, InMaxWave, InStepRotationPrice.X, InStepRotationPrice.Y);
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
		
		for (int32 i = 0; i < HexMathAxial::AxialNeighbourCount; ++i)
		{
			const FAxialCoord NeighbourCoord = Current.Coord + HexMathAxial::AxialNeighboursShifts[i];
			// Подставь свои проверки:
			// if (!IsValidCoord(N)) continue;
			if (!IsWalkable(NeighbourCoord)) continue;
			
			const int8 AngleToNeighbour = HexMathAxial::AxialNeighboursRotation[i];
			
			int RD1 = FMath::Abs(Current.Rotation - AngleToNeighbour);
			constexpr int8 LeftLimit = -3;
			constexpr int8 RightLimit = 3;
			int RD2 = 0; 
			if (AngleToNeighbour > Current.Rotation)
				RD2 = Current.Rotation - LeftLimit + RightLimit - AngleToNeighbour;
			else if (AngleToNeighbour < Current.Rotation)
				RD2 = AngleToNeighbour - LeftLimit + RightLimit - Current.Rotation;
			
			int8 RotationDiff = FMath::Min(RD1, RD2);			

			// Стоимость шага: 1 (или возьми из тайла)
			const float StepCost = InStepRotationPrice.X + RotationDiff * InStepRotationPrice.Y;// GetTraversalCost(Current.Coord, Neighbour);
			const float TentativeDistance = Current.Distance + StepCost;
			
			if (TentativeDistance > InMaxWave)
			{
				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Too far! NeighbourCoord: %s; TentativeDistance: %.2f"), 
					   *NeighbourCoord.ToString(), TentativeDistance);
				}
				continue;
			}		

			FWaveElem& Neighbour = OutPath.FindOrAdd(FWaveElem{ NeighbourCoord, AngleToNeighbour, TNumericLimits<float>::Max() });

			if (TentativeDistance < Neighbour.Distance)
			{
				Neighbour.Distance = TentativeDistance;
				Neighbour.Rotation = AngleToNeighbour;

				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Added Neighbour: %s"), *Neighbour.ToString());
				}
            	
				Open.HeapPush(Neighbour, MinHeapPred);
			}
			else if (InLog)
			{
				UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Missed Neighbour: %s"), *Neighbour.ToString());
			}
		}
	}

}