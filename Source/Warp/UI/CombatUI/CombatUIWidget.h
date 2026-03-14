// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatUIWidget.generated.h"

class UTurnOrderWidget;
class ABaseUnitActor;
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
	
	void ShowCombatUI(bool InShowCombatUI);

	void SetCurrentCombatUnits(const TArray<ABaseUnitActor*>& InCombatUnits, const int32 InActiveUnitIndex);
	void SetCurrentActiveUnitIndex(const int32 InActiveUnitIndex);
	
	void OnUnitSelected(ABaseUnitActor* InNewActiveUnit, ABaseUnitActor* InPrevActiveUnit);
	void OnUnitStartMoving(ABaseUnitActor* InNewActiveUnit);

private:
	UFUNCTION()
	void HandleNextTurnClicked();
	UFUNCTION()
	void HandleReturnToCampaignMapClicked();
	
	UPROPERTY(meta=(BindWidget))
	UButton* NextTurnButton = nullptr;
	UPROPERTY(meta=(BindWidget))
	UButton* ReturnToCampaignMapButton = nullptr;
	
	
	// UPROPERTY(EditDefaultsOnly)
	// TSubclassOf<UTurnOrderWidget> TurnOrderWidgetClass_;
	UPROPERTY(meta=(BindWidgetOptional))
	UTurnOrderWidget* TurnOrderWidget_ = nullptr;
};
