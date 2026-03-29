// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderWidget.h"

#include "MGLogs.h"
#include "TurnOrderEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/UI/CombatUI/CombatUIWidget.h"

DEFINE_LOG_CATEGORY_STATIC(UTurnOrderWidgetLog, Log, All);

void UTurnOrderWidget::Rebuild(const TArray<ABaseUnitActor*>& InCombatUnits, uint32 InNewRound)
{
	RebuildOrderListOnNewRound(InCombatUnits, InNewRound);
}

void UTurnOrderWidget::SetActiveUnit(const ABaseUnitActor* InActiveUnit, const ABaseUnitActor* InPrevUnit)
{
	RETURN_ON_FAIL(UTurnOrderWidgetLog, InActiveUnit);
	UTurnOrderEntryWidget* RowNew = UnitToRow_.FindRef(InActiveUnit);
	RowNew->SetIsCurrent(true);
	
	if (InPrevUnit != nullptr)
	{
		UTurnOrderEntryWidget* RowOld = UnitToRow_.FindRef(InPrevUnit);
		RETURN_ON_FAIL(UTurnOrderWidgetLog, RowNew);
		RowOld->SetIsCurrent(false);	
	}
}


void UTurnOrderWidget::RebuildOrderListOnNewRound(const TArray<ABaseUnitActor*>& InCombatUnits, uint32 InNewRound)
{
	RETURN_ON_FAIL(UTurnOrderWidgetLog, EntriesBox_);
	RETURN_ON_FAIL(UTurnOrderWidgetLog, tEntryWidgetClass_);
	RETURN_ON_FAIL(UTurnOrderWidgetLog, RoundText);

	EntriesBox_->ClearChildren();
	UnitToRow_.Empty();

	RoundText->SetText(FText::AsNumber(InNewRound));
	
	float MaxEntryHeight = 0.0f;
	for (int i = InCombatUnits.Num() - 1; i >= 0; --i)
	{
		UTurnOrderEntryWidget* Row = CreateWidget<UTurnOrderEntryWidget>(GetOwningPlayer(), tEntryWidgetClass_);
		
		RETURN_ON_FAIL(UTurnOrderWidgetLog, Row);
		Row->Init(InCombatUnits[i]->GetUnitType().ToString());
		
		EntriesBox_->AddChild(Row);

		if (Row->GetHeight() > MaxEntryHeight)
			MaxEntryHeight = Row->GetHeight();

		UnitToRow_.Add(InCombatUnits[i], Row);
	}

	UpdateListSize(InCombatUnits, MaxEntryHeight);
}

void UTurnOrderWidget::UpdateListSize(const TArray<ABaseUnitActor*>& InCombatUnits, float InMaxEntryHeight) const
{
	if (!EntriesSizeBox_)
	{
		return;
	}

	const int32 NumUnits = InCombatUnits.Num();
	const int32 ClampedVisibleRows = FMath::Clamp(NumUnits, MinVisibleEntries_, MaxVisibleEntries_);

	const float Height = (ClampedVisibleRows + 1) * InMaxEntryHeight;
	EntriesSizeBox_->SetHeightOverride(Height);
}

