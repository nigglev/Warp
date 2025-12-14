// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatUIWidget.generated.h"

struct FTurnOrderUnitInfo;
class UTurnOrderWidget;
class ADefaultWarpHUD;
class UBorder;
class UHorizontalBox;
class UButton;
/**
 * 
 */
UCLASS()
class WARP_API UCombatUIWidget : public UUserWidget
{
	GENERATED_BODY()

	
public:
	virtual void NativeConstruct() override;
	void Init(ADefaultWarpHUD* InHUD);
	
	void GetTurnOrderInfo(TArray<uint32>& OutUnitCombatIds, uint32& OutCurrentUnitCombatId) const;
	void GetTurnOrderUnitInfo(uint32 InUnitCombatId, FTurnOrderUnitInfo& OutInfo) const;
	
	void ShowCombatUI(bool InShowCombatUI);

private:
	void SubscribeToTurnBasedEvents();

	UFUNCTION()
	void HandleStartClicked();
	UFUNCTION()
	void HandleNextTurnClicked();
	
	void HandleTurnOrderUpdated(const TArray<uint32>& InTurnOrderUnitCombatIds, uint32 InCurrentTurnUnitCombatId);
	void HandleActiveUnitChanged(uint32 InCurrentTurnUnitCombatId);

	UPROPERTY()
	ADefaultWarpHUD* HUD_;
	
	UPROPERTY(meta=(BindWidget))
	UButton* StartButton = nullptr;
	UPROPERTY(meta=(BindWidget))
	UButton* NextTurnButton = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UTurnOrderWidget> TurnOrderWidgetClass_;
	UPROPERTY(meta=(BindWidgetOptional))
	UTurnOrderWidget* TurnOrderWidget_ = nullptr;

	TArray<uint32> TurnOrderUnitCombatIds_;
	uint32 CurrentTurnUnitCombatId_ = INDEX_NONE;


};
