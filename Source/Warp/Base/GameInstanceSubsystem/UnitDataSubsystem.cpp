// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitDataSubsystem.h"
#include "Warp/UnitStaticData/UnitStaticData.h"

void UUnitDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	if (!UnitTableAsset.IsNull())
	{
		UnitTable = UnitTableAsset.LoadSynchronous();
	}

	UnitNamesByID.Add(0, "Small");
}

const FUnitStaticData* UUnitDataSubsystem::Find(uint8 InUnitKey) const
{
	FName UnitName = UnitNamesByID[InUnitKey];
	return UnitTable ? UnitTable->FindRow<FUnitStaticData>(UnitName, TEXT("Unit lookup")) : nullptr;
}