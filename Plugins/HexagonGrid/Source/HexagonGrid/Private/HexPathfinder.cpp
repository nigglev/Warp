#include "HexPathfainer.h"

DEFINE_LOG_CATEGORY_STATIC(HexPathfinderLog, Log, Log);

FString HexMath::FPathNode::ToString() const
{
	return FString::Printf(TEXT("%s[%d] D: %.2f"), *Coord.ToString(), Rotation, Distance);
}

namespace HexMath
{
	int8 GetRotationDiff(int8 A, int8 B)
	{
		int RD1 = FMath::Abs(A - B);
		constexpr int8 LeftLimit = -3;
		constexpr int8 RightLimit = 3;
		int RD2 = 0; 
		if (B > A)
			RD2 = A - LeftLimit + RightLimit - B;
		else if (B < A)
			RD2 = B - LeftLimit + RightLimit - A;
			
		int8 RotationDiff = FMath::Min(RD1, RD2);
		return RotationDiff;
	}
	
	struct FOpenNode
	{
		FPathNode Step;
		float F = TNumericLimits<float>::Max();   // FWaveElem.Distance + h

		FOpenNode() = default;
		FOpenNode(const FAxialCoord& InCoord, int8 InRotation) : Step(InCoord, InRotation) {}
		FOpenNode(const FPathNode& InStep, float InF) : Step(InStep), F(InF) {}
	};
	
	// Min-heap по F: меньший F должен быть "наверху".
	// В UE heap-алгоритмах часто нужно инвертировать сравнение для min-heap.
	bool OpenHeapPred(const FOpenNode& A, const FOpenNode& B)
	{
		return A.F < B.F; // меньше F = выше приоритет
	};
	
	uint32 GetTypeHash(const FOpenNode& Key)
	{
		return GetTypeHash(Key.Step.Coord);
	}
	
	inline bool operator==(const FOpenNode& LHS, const FOpenNode& RHS) { return LHS.Step.Coord == RHS.Step.Coord; }
	inline bool operator!=(const FOpenNode& LHS, const FOpenNode& RHS) { return LHS.Step.Coord != RHS.Step.Coord; }
}

void HexMath::FindPathZone(const FAxialCoord& InStart, int8 InStartRotation, const FMoveParams& InMoveParams,
	TSet<FPathNode>& OutPath, bool InLog)
{
	OutPath.Reset();
	
	if (InLog)
	{
		UE_LOG(HexPathfinderLog, Log, TEXT("FindPathZone. InStart: %s; InStartRotation: %d; InMoveParams: %s"), 
			*InStart.ToString(), InStartRotation, *InMoveParams.ToString());
	}
	
	auto IsWalkable = [](const FAxialCoord& Coord) { return true; };// !Obstacles_.Contains(Coord); };
	
	auto MinHeapPred = [](const FPathNode& A, const FPathNode& B)
	{
		return A.Distance < B.Distance;
	};
	
	TArray<FPathNode> Open;
	Open.Reserve(256);
	Open.HeapPush(FPathNode{ InStart, InStartRotation, 0 }, MinHeapPred);
	
	uint32 Step = 0;
	while (Open.Num() > 0)
	{
		Step++;
    	
		FPathNode Current;
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
			
			int8 RotationDiff = GetRotationDiff(Current.Rotation, AngleToNeighbour);			

			// Стоимость шага: 1 (или возьми из тайла)
			const float StepCost = InMoveParams.MoveCost + RotationDiff * InMoveParams.RotationCost;// GetTraversalCost(Current.Coord, Neighbour);
			const float TentativeDistance = Current.Distance + StepCost;
			
			if (TentativeDistance > InMoveParams.MaxDistance)
			{
				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Too far! NeighbourCoord: %s; TentativeDistance: %.2f"), 
					   *NeighbourCoord.ToString(), TentativeDistance);
				}
				continue;
			}		

			FPathNode& Neighbour = OutPath.FindOrAdd(FPathNode(NeighbourCoord, AngleToNeighbour, TNumericLimits<float>::Max()));

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

bool HexMath::FindPath(const FAxialCoord& InStart, int8 InStartRotation, const FAxialCoord& InEnd, TOptional<int8> InEndRotation, 
	const FMoveParams& InMoveParams, TArray<FPathNode>& OutPath, bool InLog)
{
	OutPath.Reset();
	
	if (InLog)
	{
		int8 EndRotationValue = InEndRotation.IsSet() ? InEndRotation.GetValue() : TNumericLimits<int8>::Max();
		UE_LOG(HexPathfinderLog, Log, TEXT("FindPath. InStart: %s; InStartRotation: %d; InEnd: %s; EndRotationValue: %d, InMoveParams: %s"), 
			*InStart.ToString(), InStartRotation, *InEnd.ToString(), EndRotationValue, *InMoveParams.ToString());
	}
	
	// Быстрые случаи
	if (InStart == InEnd && (!InEndRotation.IsSet() || InEndRotation.GetValue() == InStartRotation))
	{
		OutPath.Add(FPathNode{ InStart, InStartRotation, 0 });
		return true;
	}
	
	auto IsWalkable = [](const FAxialCoord& Coord) { return true; };// !Obstacles_.Contains(Coord); };
	
	if (!IsWalkable(InStart) || !IsWalkable(InEnd)) return false;
	
	TArray<FOpenNode> Open;
	Open.Reserve(256);

	TMap<FAxialCoord, FPathNode> CameFrom;
	CameFrom.Reserve(256);

	TSet<FOpenNode> Passed;
	Passed.Reserve(256);
	
	{
		float H = AxialDistance(InStart, InEnd) * InMoveParams.MoveCost;
		Open.HeapPush(FOpenNode(FPathNode(InStart, InStartRotation, 0), H), OpenHeapPred);
	}
	
	uint32 Step = 0;
	while (Open.Num() > 0)
	{
		Step++;
    	
		FOpenNode Current;
		Open.HeapPop(Current, OpenHeapPred, EAllowShrinking::No);
    	
		if (InLog)
		{
			UE_LOG(HexPathfinderLog, Log, TEXT("\t %u: OpenNim: %d; Step: %s; F: %.2f"), Step, Open.Num(), *Current.Step.ToString(), Current.F);
		}
		
		if (Current.Step.Coord == InEnd)
		{
			if (InEndRotation.IsSet())
			{
				float RotationDist = HexMath::GetRotationDiff(Current.Step.Rotation, InEndRotation.GetValue()) * InMoveParams.RotationCost;
				float RestDist = InMoveParams.MaxDistance - Current.Step.Distance;
				if (RestDist < RotationDist)
				{
					if (InLog)
					{
						UE_LOG(HexPathfinderLog, Log, TEXT("End Rotation is too far!"));
					}
					return false;
				}
			}
			
			// Восстановление пути
			FPathNode C = Current.Step;
			OutPath.Add(C);

			while (C.Coord != InStart)
			{
				FPathNode* Parent = CameFrom.Find(C.Coord);
				if (Parent == nullptr)
				{
					UE_LOG(HexPathfinderLog, Error, TEXT("Return back Path was broken!"));
					OutPath.Reset();
					return false;
				}
				C = *Parent;
				OutPath.Add(C);
			}

			Algo::Reverse(OutPath);
			return true;
		}
		
		for (int32 i = 0; i < HexMathAxial::AxialNeighbourCount; ++i)
		{
			const FAxialCoord NeighbourCoord = Current.Step.Coord + HexMathAxial::AxialNeighboursShifts[i];
			if (!IsWalkable(NeighbourCoord)) continue;
			
			const int8 AngleToNeighbour = HexMathAxial::AxialNeighboursRotation[i];
			
			int8 RotationDiff = GetRotationDiff(Current.Step.Rotation, AngleToNeighbour);			

			// Стоимость шага: 1 (или возьми из тайла)
			const float StepCost = InMoveParams.MoveCost + RotationDiff * InMoveParams.RotationCost;// GetTraversalCost(Current.Coord, Neighbour);
			const float TentativeDistance = Current.Step.Distance + StepCost;
			
			if (TentativeDistance > InMoveParams.MaxDistance)
			{
				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Too far! NeighbourCoord: %s; TentativeDistance: %.2f"), 
					   *NeighbourCoord.ToString(), TentativeDistance);
				}
				continue;
			}
			
			const int32 H = AxialDistance(NeighbourCoord, InEnd) * InMoveParams.MoveCost;
			const float TentativeF = TentativeDistance + H;

			FOpenNode& Neighbour = Passed.FindOrAdd(FOpenNode(NeighbourCoord, AngleToNeighbour));

			if (TentativeF < Neighbour.F)
			{
				Neighbour.Step.Rotation = AngleToNeighbour;
				Neighbour.Step.Distance = TentativeDistance;
				Neighbour.F = TentativeF;

				if (InLog)
				{
					UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Added Neighbour: %s; Neighbour F: %.2f"), 
						*Neighbour.Step.ToString(), Neighbour.F);
				}
            	
				Open.HeapPush(Neighbour, OpenHeapPred);
				CameFrom.Add(Neighbour.Step.Coord, Current.Step);
			}
			else if (InLog)
			{
				UE_LOG(HexPathfinderLog, Log, TEXT("\t\t Missed Neighbour: %s; Neighbour F: %.2f"), 
					*Neighbour.Step.ToString(), Neighbour.F);
			}
		}
	}
	return false;
}
