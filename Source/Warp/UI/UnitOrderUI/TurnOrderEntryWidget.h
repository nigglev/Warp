// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderEntryWidget.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API UTurnOrderEntryWidget : public UUserWidget
{
	GENERATED_BODY()

	
public:
	void Init(int32 InUnitId, class ATurnBasedSystem* InManager);

	void SetIsCurrent(bool bInCurrent);
	int32 GetUnitId() const { return UnitId; }

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UImage> UnitImage_;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UTextBlock> UnitNameText_;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> BackgroundBorder_;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Style")
	FLinearColor AllyColor_ = FLinearColor(0.f, 0.5f, 0.f, 0.7f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Style")
	FLinearColor EnemyColor_ = FLinearColor(0.5f, 0.f, 0.f, 0.7f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Style")
	FLinearColor CurrentOutlineColor_ = FLinearColor::Yellow;

private:

	int32 UnitId = INDEX_NONE;
	bool bIsAlly = false;
};
