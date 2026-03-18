// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TurnOrderEntryWidget.generated.h"

class USizeBox;
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
	void Init(const FString& InUnitName);
	void SetIsCurrent(bool bInCurrent) const;

	float GetWidth();
	float GetHeight();
protected:
	UPROPERTY(meta = (BindWidget))
	UMultiLineEditableText* UnitNameText = nullptr;
	UPROPERTY(meta = (BindWidget))
	UImage* ActiveUnitSignImage = nullptr;
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> EntrySizeBox = nullptr;
	UPROPERTY(meta = (BindWidget))
	UBorder* BackgroundBorder = nullptr;

	const FLinearColor ActiveColor   = FLinearColor(1.f, 0.85f, 0.2f, 1.f);
	const FLinearColor InactiveColor = FLinearColor(1.f, 1.f, 1.f, 1.f);
};
