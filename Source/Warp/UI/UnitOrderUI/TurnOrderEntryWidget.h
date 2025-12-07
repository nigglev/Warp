// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderEntryWidget.generated.h"

struct FTurnOrderUnitInfo;
class UTextBlock;
class UBorder;
/**
 * 
 */
UCLASS()
class WARP_API UTurnOrderEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Init(uint32 InUnitCombatID, const FTurnOrderUnitInfo& Info, bool bIsCurrent);
	void SetIsCurrent(bool bInCurrent);

	int32 GetUnitId() const { return UnitCombatId_; }

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* UnitNameText;
	UPROPERTY(meta = (BindWidget))
	UBorder* BackgroundBorder;
	
	FLinearColor AllyColor = FLinearColor(0.f, 0.5f, 0.f, 0.7f);
	FLinearColor EnemyColor = FLinearColor(0.5f, 0.f, 0.f, 0.7f);

private:
	int32 UnitCombatId_ = INDEX_NONE;
	bool bIsAlly_ = false;
};
