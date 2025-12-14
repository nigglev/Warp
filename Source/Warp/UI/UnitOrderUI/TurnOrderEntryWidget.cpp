// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderEntryWidget.h"

#include "TurnOrderUnitInfo.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UTurnOrderEntryWidget::Init(uint32 InUnitCombatID, const FTurnOrderUnitInfo& Info, bool bIsCurrent)
{
	UnitCombatId_ = InUnitCombatID;
	bIsAlly_ = Info.bIsAlly_;

	if (UnitNameText)
	{
		const FString NameStr = FString::Printf(
			TEXT("%s %d"),
			*Info.UnitTypeName_.ToString(),
			UnitCombatId_
		);
		UnitNameText->SetText(FText::FromString(NameStr));
		UnitNameText->SetColorAndOpacity(FSlateColor(bIsAlly_ ? AllyColor : EnemyColor));
	}

	SetIsCurrent(bIsCurrent);
}

void UTurnOrderEntryWidget::SetIsCurrent(bool bInCurrent)
{
	if (bInCurrent)
		CurrenTurnImage->SetVisibility(ESlateVisibility::Visible);
	else
		CurrenTurnImage->SetVisibility(ESlateVisibility::Hidden);
}