// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DefaultWarpHUD.generated.h"

class UEndTurnWidget;
class UCombatUIWidget;
class AWarpGameState;
class UTurnBasedSystemManager;
class UUnitSpawnWidget;
class STurnOrderWidget;
/**
 * 
 */
UCLASS()
class WARP_API ADefaultWarpHUD : public AHUD
{
	GENERATED_BODY()

	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UCombatUIWidget* GetCombatUI() const;
	
protected:
	APlayerController* Init() const;
	
	AWarpGameState* GetGameState() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCombatUIWidget> CombatUIWidgetClass;
	UPROPERTY()
	UCombatUIWidget* CombatUIWidget = nullptr;
};
