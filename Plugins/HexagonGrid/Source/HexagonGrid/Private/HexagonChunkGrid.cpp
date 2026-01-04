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

void UHexagonChunkGrid::CreateNewChunks(const FVector& InNewPosition)
{
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	TOptional<FOffsetCoord> ObserverChunkCoordOp = WorldToChunkCoord(InNewPosition);
	if (!ObserverChunkCoordOp.IsSet())
		return;
	
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (!ensure(Settings->HexGridActorClass_ != nullptr))
		return;
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (!ensure(CDO != nullptr))
		return;

	float HexSize = CDO->GetHexSize();
	uint32 NumColsRows = CDO->GetGridSize();

	FOffsetRealCoord ChunkPitch = GetChunkPitchNoGap<HEX_LAYOUT>(NumColsRows, NumColsRows, HexSize);
	
	FOffsetCoord ObserverChunkCoord = ObserverChunkCoordOp.GetValue();
	
	if (CurrentChunkCoord_ != ObserverChunkCoord)
	{
		CurrentChunkCoord_ = ObserverChunkCoord;
		
		int32 iT = Settings->BuildChunkAround;
		
		for (int64 Idx = CurrentChunkCoord_.Up - iT; Idx <= CurrentChunkCoord_.Up + iT; ++Idx)
			for (int64 Jdx = CurrentChunkCoord_.Right - iT; Jdx <= CurrentChunkCoord_.Right + iT; ++Jdx)
			{
				FOffsetCoord ChunkCoord(Jdx, Idx);
				int32 ChunkIndex = FindChunkIndex(ChunkCoord);
				bool bAlreadyPresent = ChunkIndex != INDEX_NONE;
				
				if (!bAlreadyPresent)
				{
					FOffsetRealCoord NewChunkPos(ChunkCoord.Right * ChunkPitch.Right,  ChunkCoord.Up * ChunkPitch.Up);
				
					FVector ChunkPos = OffsetHexToWorld(NewChunkPos);

					AHexGridISMActor* HexActor = Cast<AHexGridISMActor>(GetWorld()->SpawnActor(Settings->HexGridActorClass_, &ChunkPos));
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
	// TOptional<HexMath::FOffsetCoord> ChunkCoordOp = WorldToChunkCoord(InPosition);
	// if (!ChunkCoordOp.IsSet())
	// 	return;
	//
	// int32 ChunkIndex = FindChunkIndex(ChunkCoordOp.GetValue());
	// if (ChunkIndex == INDEX_NONE)
	// 	return;
	//
	// FChunkData& ChunkData = ChunksList_[ChunkIndex];
	//
	// if (!ensure(ChunkData.ChunkActor != nullptr))
	// 	return;
	
	//ChunkData.ChunkActor->SelectCell(InPosition);
	
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (!ensure(Settings->HexGridActorClass_ != nullptr))
		return;
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (!ensure(CDO != nullptr))
		return;
	
	float HexSize = CDO->GetHexSize();
	uint32 NumColsRows = CDO->GetGridSize();
	
	FVector LocalPosition = InPosition;
	HexMath::FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(LocalPosition, HexSize);
	
	HexMath::FOffsetCoord OCoord = HexMath::HexMathAxial::WorldToOffset<HEX_LAYOUT>(Coord, HexSize);

	int64 Rc = OCoord.Right / NumColsRows;
	int64 Ri = OCoord.Right % NumColsRows;
	if (Ri < 0) Rc--;
	
	int64 Upc = OCoord.Up / NumColsRows;
	int64 Upi = OCoord.Up % NumColsRows;
	if (Upi < 0) Upc--;
	
	HexMath::FOffsetCoord ChunkCoord(Rc,Upc);
	
	UE_LOG(HexGridLog, Warning, TEXT("ChunkCoord: %s; GlobalORCoord: %s; GlobalOCoord: %s"), 
		*ChunkCoord.ToString(), *Coord.ToString(), *OCoord.ToString());
	
	int32 ChunkIndex = FindChunkIndex(ChunkCoord);
	if (ChunkIndex != INDEX_NONE)
	{
		ChunksList_[ChunkIndex].ChunkActor->SelectCell(InPosition);
	}
}

TOptional<HexMath::FOffsetCoord> UHexagonChunkGrid::WorldToChunkCoord(const FVector& InWorldPoint)
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (!ensure(Settings->HexGridActorClass_ != nullptr))
		return TOptional<HexMath::FOffsetCoord>();
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (!ensure(CDO != nullptr))
		return TOptional<HexMath::FOffsetCoord>();
	
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CDO->GetHexSize();
	uint32 NumColsRows = CDO->GetGridSize();

	FOffsetRealCoord FC = WorldToOffsetHex(InWorldPoint);
		
	FOffsetRealCoord ChunkPitch = GetChunkPitchNoGap<HEX_LAYOUT>(NumColsRows, NumColsRows, HexSize);
	
	FOffsetCoord ObserverChunkCoord(FMath::FloorToInt(FC.Right / ChunkPitch.Right), FMath::FloorToInt(FC.Up / ChunkPitch.Up));
	
	return ObserverChunkCoord;
}
