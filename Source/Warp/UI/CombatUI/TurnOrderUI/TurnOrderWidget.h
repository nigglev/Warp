// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderWidget.generated.h"

class UTextBlock;
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
	void Rebuild(const TArray<ABaseUnitActor*>& InCombatUnits, uint32 InNewRound);
	void SetActiveUnit(const ABaseUnitActor* InActiveUnit, const ABaseUnitActor* InPrevUnit);

protected:
	void RebuildOrderListOnNewRound(const TArray<ABaseUnitActor*>& InCombatUnits, uint32 InNewRound);
	void UpdateListSize(const TArray<ABaseUnitActor*>& InCombatUnits, float InMaxEntryHeight) const;
	
	UPROPERTY(EditAnywhere, Category="Turn Order|Sizing", meta=(ClampMin="1"))
	int32 MinVisibleEntries_ = 3;

	UPROPERTY(EditAnywhere, Category="Turn Order|Sizing", meta=(ClampMin="1"))
	int32 MaxVisibleEntries_ = 10;

	UPROPERTY(EditDefaultsOnly, Category="Turn Order")
	TSubclassOf<UTurnOrderEntryWidget> tEntryWidgetClass_;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UScrollBox> EntriesBox_;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoundText = nullptr;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> EntriesSizeBox_ = nullptr;

	TMap<ABaseUnitActor*, UTurnOrderEntryWidget*> UnitToRow_;
	ABaseUnitActor* ActiveUnit_ = nullptr;

};

