// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderEntryWidget.h"

#include "TurnOrderUnitInfo.h"
#include "Components/Border.h"
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
	}

	if (BackgroundBorder)
	{
		BackgroundBorder->SetBrushColor(bIsAlly_ ? AllyColor : EnemyColor);
	}

	SetIsCurrent(bIsCurrent);
}

void UTurnOrderEntryWidget::SetIsCurrent(bool bInCurrent)
{
	if (!BackgroundBorder)
		return;

	const FLinearColor BaseColor = bIsAlly_ ? AllyColor : EnemyColor;
	FLinearColor FinalColor = BaseColor;

	if (bInCurrent)
	{
		FinalColor.A = 1.0f;
	}

	BackgroundBorder->SetBrushColor(FinalColor);
}