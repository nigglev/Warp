// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "UnitDataSubsystem.generated.h"

struct FUnitStaticData;
/**
 * 
 */
UCLASS(Config=Game)
class WARP_API UUnitDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	const FUnitStaticData* Find(uint8 InUnitKey) const;

	UPROPERTY(EditDefaultsOnly, Config, Category="Units", meta=(AllowedClasses="DataTable"))
	TSoftObjectPtr<UDataTable> UnitTableAsset;

private:
	UPROPERTY()
	UDataTable* UnitTable = nullptr;
	UPROPERTY()
	TMap<uint8, FName> UnitNamesByID;
};
