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

	void GetTurnOrderInfo(TArray<uint32>& OutUnitCombatIds, uint32& OutCurrentUnitCombatId) const;
	void GetTurnOrderUnitInfo(uint32 InUnitCombatId, FTurnOrderUnitInfo& OutInfo) const;
	
protected:
	APlayerController* Init() const;
	void SetupWidgets(APlayerController* InPC);
	void SetupTBSMEvents();
	
	AWarpGameState* GetGameState() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;
		
	void HandleTurnOrderUpdated(const TArray<uint32>& InTurnOrderUnitCombatIds, uint32 InCurrentTurnUnitCombatId);
	void HandleActiveUnitChanged(uint32 InCurrentTurnUnitCombatId);

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UCombatUIWidget> CombatUIWidgetClass_;
	UPROPERTY()
	UCombatUIWidget* CombatUIWidget_ = nullptr;
	
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UTurnOrderWidget> TurnOrderWidgetClass_;
	UPROPERTY()
	UTurnOrderWidget* TurnOrderWidget_ = nullptr;
	
	TArray<uint32> TurnOrderUnitCombatIds_;
	uint32 CurrentTurnUnitCombatId_ = INDEX_NONE;

};


