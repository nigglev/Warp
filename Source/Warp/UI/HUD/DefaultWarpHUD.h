// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DefaultWarpHUD.generated.h"

struct FTurnOrderUnitInfo;
class UEndTurnWidget;
class UCombatUIWidget;
class UTurnOrderWidget;
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

	AWarpGameState* GetGameState() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;
	
protected:
	APlayerController* Init() const;
	void SetupWidgets(APlayerController* InPC);

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCombatUIWidget> CombatUIWidgetClass_;
	UPROPERTY()
	UCombatUIWidget* CombatUIWidget_ = nullptr;
	

	
	

};


