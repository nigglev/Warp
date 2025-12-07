// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderWidget.generated.h"

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
	void Init(ADefaultWarpHUD* InHUD);

	void RebuildFromHUD();
	void UpdateCurrentFromHUD();

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UTurnOrderEntryWidget> EntryWidgetClass_;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UVerticalBox> EntriesBox_;
	
	UPROPERTY()
	ADefaultWarpHUD* HUD_;
	UPROPERTY()
	TArray<UTurnOrderEntryWidget*> EntryWidgets_;
};
