// Fill out your copyright notice in the Description page of Project Settings.


#include "GameAssets.h"

#include "MGLogs.h"
#include "UnitStaticData/UnitDataTableRows.h"

DEFINE_LOG_CATEGORY_STATIC(AGameAssetsLog, Log, All);

UGameAssets::UGameAssets(const FObjectInitializer& ObjectInitializer)
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Warp Game Assets");
}

TSubclassOf<ABaseUnitActor> UGameAssets::GetUnitActorClass(const FName& InUnitType) const
{
	RETURN_ON_FAIL_NULL(AGameAssetsLog, UnitActorsTable_ != nullptr);
	
	FUnitActorsTableRows* Row = UnitActorsTable_->FindRow<FUnitActorsTableRows>(InUnitType, FString(__FUNCDNAME__));
	RETURN_ON_FAIL_NULL_T(AGameAssetsLog, Row != nullptr, TEXT("Couldn't find Actor Class for %s"), *InUnitType.ToString());
	
	return Row->UnitActor.LoadSynchronous();
}
