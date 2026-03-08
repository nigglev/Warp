// Fill out your copyright notice in the Description page of Project Settings.


#include "GameSettings.h"

#include "MGLogs.h"
#include "UnitStaticData/UnitDataTableRows.h"

DEFINE_LOG_CATEGORY_STATIC(AGameSettingsLog, Log, All);

UGameSettings::UGameSettings(const FObjectInitializer& ObjectInitializer)
{
	CategoryName = TEXT("Game");
	SectionName = TEXT("Warp Game Assets");
}

TSubclassOf<ABaseUnitActor> UGameSettings::GetUnitActorClass(const FName& InUnitType, bool InGhost) const
{
	UDataTable* DT = InGhost ? UnitGhostActorsTable_.LoadSynchronous() : UnitActorsTable_.LoadSynchronous();;
	RETURN_ON_FAIL_NULL(AGameSettingsLog, DT != nullptr);
	
	FUnitActorsTableRows* Row = DT->FindRow<FUnitActorsTableRows>(InUnitType, FString(__FUNCDNAME__));
	RETURN_ON_FAIL_NULL_T(AGameSettingsLog, Row != nullptr, TEXT("Couldn't find Actor Class for %s"), *InUnitType.ToString());
	
	return Row->UnitActor.LoadSynchronous();
}
