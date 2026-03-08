// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderWidget.generated.h"

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
	UPROPERTY(EditAnywhere)
	TSubclassOf<UTurnOrderEntryWidget> EntryWidgetClass_;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> EntriesBox_;
	UPROPERTY()
	TArray<UTurnOrderEntryWidget*> EntryWidgets_;
};
