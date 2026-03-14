// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderWidget.generated.h"

class USizeBox;
class ABaseUnitActor;
class UCombatUIWidget;
class UScrollBox;
class UTurnOrderEntryWidget;
/**
 * 
 */
UCLASS()
class WARP_API UTurnOrderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void RebuildFromHUD(const TArray<ABaseUnitActor*>& InCombatUnits, const int32 InActiveUnitIndex);
	void UpdateCurrentFromHUD(const int32 InActiveUnitIndex);

protected:
	void UpdateListSize();

	UPROPERTY(EditAnywhere, Category="Turn Order|Sizing")
	int32 MinVisibleEntries_ = 3;

	UPROPERTY(EditAnywhere, Category="Turn Order|Sizing")
	int32 MaxVisibleEntries_ = 10;

	UPROPERTY(EditAnywhere, Category="Turn Order|Sizing", meta=(ClampMin="0.0"))
	float ExtraHeightPadding_ = 0.f;
	
	UPROPERTY(EditDefaultsOnly, Category="Turn Order")
	TSubclassOf<UTurnOrderEntryWidget> tEntryWidgetClass_;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> EntriesSizeBox_ = nullptr;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> EntriesBox_;
	UPROPERTY()
	TArray<UTurnOrderEntryWidget*> EntryWidgets_;
};
