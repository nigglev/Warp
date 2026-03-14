// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderEntryWidget.h"

#include "MGLogs.h"
#include "Components/EditableTextBox.h"
#include "Components/Image.h"
#include "Components/MultiLineEditableText.h"

DEFINE_LOG_CATEGORY_STATIC(UTurnOrderWidgetEntryLog, Log, All);

void UTurnOrderEntryWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UTurnOrderEntryWidget::Init(const FString& InUnitName, bool bIsCurrent)
{
	if (UnitNameText)
	{
		const FString NameStr = FString::Printf(TEXT("%s"), *InUnitName);
		UnitNameText->SetText(FText::FromString(NameStr));
	}

	SetIsCurrent(bIsCurrent);
}

void UTurnOrderEntryWidget::SetIsCurrent(bool bInCurrent) const
{
	RETURN_ON_FAIL(UTurnOrderWidgetEntryLog, ActiveUnitSignImage);
	if (bInCurrent)
		ActiveUnitSignImage->SetVisibility(ESlateVisibility::Visible);
	else
		ActiveUnitSignImage->SetVisibility(ESlateVisibility::Hidden);
}
