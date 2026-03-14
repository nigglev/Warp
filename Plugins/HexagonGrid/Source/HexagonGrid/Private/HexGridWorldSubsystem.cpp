// Fill out your copyright notice in the Description page of Project Settings.

#include "HexGridWorldSubsystem.h"
#include "HexagonChunkGrid.h"
#include "HexagonGridSettings.h"
#include "HexGridISMActor.h"
#include "MoveParams.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridWSLog, Log, Log);

// UHexGridWorldSubsystem* UHexGridWorldSubsystem::Get(const UObject* InWorldContextObject)
// {
// 	return InWorldContextObject->GetWorld()->GetSubsystem<UHexGridWorldSubsystem>();
// }

void UHexGridWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	ChunkGrid_ = NewObject<UHexagonChunkGrid>(this);
}

void UHexGridWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
	ChunkGrid_ = nullptr;
}

void UHexGridWorldSubsystem::OnChangeObserverPosition(const FVector& InNewPosition)
{
	ChunkGrid_->OnChangeObserverPosition(InNewPosition);
}

HexMath::FAxialCoord UHexGridWorldSubsystem::WorldToAxialCellCoord(const FVector& InWorldPoint)
{
	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = GetHexSize();
	
	FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(InWorldPoint, HexSize);
	
	FAxialCoord CellCoord = HexMathAxial::OffsetToAxial<HEX_LAYOUT>(Coord, HexSize);
	
	return CellCoord;
}

FVector UHexGridWorldSubsystem::AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset)
{
	return AxialCellToWorldCoord(InAxialCoord, GetHexSize(), InZOffset);
}

FVector UHexGridWorldSubsystem::AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InHexSize,
	float InZOffset)
{
	using namespace HexMath;
	using namespace HexMath::HexMathAxial;
	using namespace HexMath::HexMathOffset;
	
	FOffsetCoord OffsetCoord = AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	FOffsetRealCoord FC = GetHexOffsetPos<HEX_LAYOUT>(OffsetCoord, InHexSize);
	const FVector Loc = HexToWorldSnapped<HEX_LAYOUT>(FC, InHexSize, InZOffset);
	
	return Loc;
}

float UHexGridWorldSubsystem::GetHexSize()
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (Settings->HexGridActorClass_ == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("Settings::HexGridActorClass is not set!"));
		return 1;
	}
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (CDO == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("HexGridActor CDO is not valid!"));
		return 1;
	}
	
	float HexSize = CDO->GetHexSize();
	return HexSize;
}

void UHexGridWorldSubsystem::GetFlatTopHexCorners(const FVector& C, float HexSize, TArray<FVector>& OutCorners)
{
	return HexMath::HexMathAxial::GetFlatTopHexCorners<HEX_LAYOUT>(C, HexSize, OutCorners);
}

HexMath::FAxialCoord UHexGridWorldSubsystem::TransformCell(const HexMath::FAxialCoord& InAxialBaseCoord, int8 InRotation, const HexMath::FOffsetCoord& InLocalShift)
{
	HexMath::FAxialCoord LocalShift = HexMath::HexMathAxial::OffsetToAxial<HEX_LAYOUT>(InLocalShift);
	HexMath::FAxialCoord NewPos = HexMath::HexMathAxial::Transform(InAxialBaseCoord, InRotation, LocalShift);
	return NewPos;
}
