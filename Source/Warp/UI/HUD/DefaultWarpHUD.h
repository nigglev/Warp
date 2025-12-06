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

	UCombatUIWidget* GetCombatUI() const;

	void GetTurnOrder(TArray<uint32>& OutUnitIds, uint32& OutCurrentUnitId) const;
	bool GetTurnOrderUnitInfo(FTurnOrderUnitInfo& OutInfo) const;
	
protected:
	APlayerController* Init() const;
	void SetupWidgets(APlayerController* InPC);
	void SetupTBSMEvents();
	
	AWarpGameState* GetGameState() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;
		
	void HandleTurnOrderUpdated(const TArray<uint32>& InTurnOrderUnitIds, uint32 InCurrentTurnUnitId);
	void HandleActiveUnitChanged(uint32 InCurrentTurnUnitId);

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCombatUIWidget> CombatUIWidgetClass;
	UPROPERTY()
	UCombatUIWidget* CombatUIWidget = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UTurnOrderWidget> TurnOrderWidgetClass;
	UPROPERTY()
	UTurnOrderWidget* TurnOrderWidget = nullptr;
	
	TArray<uint32> CachedTurnOrderUnitIds_;
	uint32 CachedCurrentTurnUnitId_ = INDEX_NONE;

};


