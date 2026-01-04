// Fill out your copyright notice in the Description page of Project Settings.

#include "HexGridWorldSubsystem.h"
#include "HexagonChunkGrid.h"

DEFINE_LOG_CATEGORY_STATIC(HexGridWSLog, Log, Log);

UHexGridWorldSubsystem* UHexGridWorldSubsystem::Get(const UObject* InWorldContextObject)
{
	return InWorldContextObject->GetWorld()->GetSubsystem<UHexGridWorldSubsystem>();
}

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

void UHexGridWorldSubsystem::SelectCell(const FVector& InPosition)
{
	ChunkGrid_->SelectCell(InPosition);
}
