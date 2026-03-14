// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderEntryWidget.generated.h"

class UMultiLineEditableText;
class UMultiLineEditableTextBox;
class UImage;
class UBorder;
class UTextBlock;
/**
 * 
 */
UCLASS()
class WARP_API UTurnOrderEntryWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeConstruct() override;
	void Init(const FString& InUnitName, bool bIsCurrent);
	void SetIsCurrent(bool bInCurrent) const;

protected:
	UPROPERTY(meta = (BindWidget))
	UMultiLineEditableText* UnitNameText = nullptr;
	UPROPERTY(meta = (BindWidget))
	UImage* ActiveUnitSignImage = nullptr;
};
