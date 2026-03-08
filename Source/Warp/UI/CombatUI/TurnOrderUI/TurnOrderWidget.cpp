// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderWidget.h"

#include "MGLogs.h"
#include "TurnOrderEntryWidget.h"
#include "Components/ScrollBox.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/UI/CombatUI/CombatUIWidget.h"

DEFINE_LOG_CATEGORY_STATIC(UTurnOrderWidgetLog, Log, All);

void UTurnOrderWidget::RebuildFromHUD(const TArray<ABaseUnitActor*>& InCombatUnits, const int32 InActiveUnitIndex)
{
	RETURN_ON_FAIL(UTurnOrderWidgetLog, EntriesBox_);
	RETURN_ON_FAIL(UTurnOrderWidgetLog, EntryWidgetClass_);

	EntriesBox_->ClearChildren();
	EntryWidgets_.Reset();

	for (int i = 0; i < InCombatUnits.Num(); ++i)
	{
		UTurnOrderEntryWidget* Row =
			CreateWidget<UTurnOrderEntryWidget>(GetWorld(), EntryWidgetClass_);
		RETURN_ON_FAIL(UTurnOrderWidgetLog, Row);

		bool bIsCurrent = i == InActiveUnitIndex;
		Row->Init(InCombatUnits[i]->GetUnitType().ToString(), bIsCurrent);
		
		EntriesBox_->AddChild(Row);
		EntryWidgets_.Add(Row);
	}
}

void UTurnOrderWidget::UpdateCurrentFromHUD(const int32 InActiveUnitIndex)
{
	for (int i = 0; i < EntryWidgets_.Num(); ++i)
	{
		UTurnOrderEntryWidget* Row = EntryWidgets_[i];
		RETURN_ON_FAIL(UTurnOrderWidgetLog, Row);

		bool bIsCurrent = i == InActiveUnitIndex;
		Row->SetIsCurrent(bIsCurrent);
	}
}