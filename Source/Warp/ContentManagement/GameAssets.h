// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameAssets.generated.h"

class ABaseUnitActor;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Warp Game Assets"))
class WARP_API UGameAssets : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UGameAssets(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static UGameAssets* Get() 
	{ 
		return GetMutableDefault<UGameAssets>();
	}
	
	TSubclassOf<ABaseUnitActor> GetUnitActorClass(const FName& InUnitType, bool InGhost = false) const;
	
protected:
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UDataTable> UnitActorsTable_;
	
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UDataTable> UnitGhostActorsTable_;
};
