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

TOptional<HexMath::FAxialCoord> UHexGridWorldSubsystem::WorldToAxialCellCoord(const FVector& InWorldPoint)
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (Settings->HexGridActorClass_ == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("Settings::HexGridActorClass is not set!"));
		return {};
	}
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (CDO == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("HexGridActor CDO is not valid!"));
		return {};
	}

	using namespace HexMath;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CDO->GetHexSize();
	
	FOffsetRealCoord Coord = HexMath::HexMathOffset::WorldToHexSnapped<HEX_LAYOUT>(InWorldPoint, HexSize);
	
	FAxialCoord CellCoord = HexMathAxial::OffsetToAxial<HEX_LAYOUT>(Coord, HexSize);
	
	return CellCoord;
}

TOptional<FVector> UHexGridWorldSubsystem::AxialCellToWorldCoord(const HexMath::FAxialCoord& InAxialCoord, float InZOffset/* = 0*/)
{
	UHexagonGridSettings* Settings = UHexagonGridSettings::Get();
	if (Settings->HexGridActorClass_ == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("Settings::HexGridActorClass is not set!"));
		return {};
	}
	
	AHexGridISMActor* CDO = Cast<AHexGridISMActor>(Settings->HexGridActorClass_->ClassDefaultObject);
	if (CDO == nullptr)
	{
		UE_LOG(HexGridWSLog, Error, TEXT("HexGridActor CDO is not valid!"));
		return {};
	}

	using namespace HexMath;
	using namespace HexMath::HexMathAxial;
	using namespace HexMath::HexMathOffset;
	
	float HexSize = CDO->GetHexSize();
	
	FOffsetCoord OffsetCoord = AxialToOffset<HEX_LAYOUT>(InAxialCoord);
	FOffsetRealCoord FC = GetHexOffsetPos<HEX_LAYOUT>(OffsetCoord, HexSize);
	const FVector Loc = HexToWorldSnapped<HEX_LAYOUT>(FC, HexSize, InZOffset);
	
	return Loc;
}
