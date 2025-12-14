// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderWidget.generated.h"

class UScrollBox;
class UCombatUIWidget;
class UVerticalBox;
class UTurnOrderEntryWidget;
class ADefaultWarpHUD;
/**
 * 
 */
UCLASS()
class WARP_API UTurnOrderWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void Init(UCombatUIWidget* InCombatWidgetOwner);

	void RebuildFromHUD();
	void UpdateCurrentFromHUD();

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UTurnOrderEntryWidget> EntryWidgetClass_;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> EntriesBox_;
	
	UPROPERTY()
	UCombatUIWidget* CombatWidgetOwner_;
	UPROPERTY()
	TArray<UTurnOrderEntryWidget*> EntryWidgets_;
};
