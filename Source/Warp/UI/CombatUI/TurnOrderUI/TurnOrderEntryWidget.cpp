// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderEntryWidget.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UTurnOrderEntryWidget::Init(const FString& InUnitName, bool bIsCurrent)
{
	if (UnitNameText)
	{
		const FString NameStr = FString::Printf(TEXT("%s"), *InUnitName);
		UnitNameText->SetText(FText::FromString(NameStr));
		UnitNameText->SetColorAndOpacity(FSlateColor(bIsAlly_ ? AllyColor : EnemyColor));
	}

	SetIsCurrent(bIsCurrent);
}

void UTurnOrderEntryWidget::SetIsCurrent(bool bInCurrent)
{
	if (bInCurrent)
		ActiveUnitSignImage->SetVisibility(ESlateVisibility::Visible);
	else
		ActiveUnitSignImage->SetVisibility(ESlateVisibility::Hidden);
}