// Fill out your copyright notice in the Description page of Project Settings.


#include "TurnOrderWidget.h"

#include "MGLogs.h"
#include "TurnOrderEntryWidget.h"
#include "TurnOrderUnitInfo.h"
#include "Components/VerticalBox.h"
#include "Warp/UI/CombatUI/CombatUIWidget.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"

DEFINE_LOG_CATEGORY_STATIC(UTurnOrderWidgetLog, Log, All);

void UTurnOrderWidget::Init(UCombatUIWidget* InCombatWidgetOwner)
{
	CombatWidgetOwner_ = InCombatWidgetOwner;
}

void UTurnOrderWidget::RebuildFromHUD()
{
	RETURN_ON_FAIL(UTurnOrderWidgetLog, CombatWidgetOwner_);
	RETURN_ON_FAIL(UTurnOrderWidgetLog, EntriesBox_);
	RETURN_ON_FAIL(UTurnOrderWidgetLog, EntryWidgetClass_);

	TArray<uint32> UnitCombatIds;
	uint32 CurrentUnitCombatId = INDEX_NONE;
	CombatWidgetOwner_->GetTurnOrderInfo(UnitCombatIds, CurrentUnitCombatId);

	EntriesBox_->ClearChildren();
	EntryWidgets_.Reset();

	for (uint32 UnitCombatId : UnitCombatIds)
	{
		FTurnOrderUnitInfo Info;
		CombatWidgetOwner_->GetTurnOrderUnitInfo(UnitCombatId, Info);
		
		UTurnOrderEntryWidget* Row =
			CreateWidget<UTurnOrderEntryWidget>(GetWorld(), EntryWidgetClass_);
		RETURN_ON_FAIL(UTurnOrderWidgetLog, Row);

		bool bIsCurrent = (UnitCombatId == CurrentUnitCombatId);
		Row->Init(UnitCombatId, Info, bIsCurrent);

		EntriesBox_->AddChild(Row);
		EntryWidgets_.Add(Row);
	}
}

void UTurnOrderWidget::UpdateCurrentFromHUD()
{
	RETURN_ON_FAIL(UTurnOrderWidgetLog, CombatWidgetOwner_);

	TArray<uint32> UnitCombatIds;
	uint32 CurrentUnitCombatId = INDEX_NONE;
	CombatWidgetOwner_->GetTurnOrderInfo(UnitCombatIds, CurrentUnitCombatId);

	for (UTurnOrderEntryWidget* Row : EntryWidgets_)
	{
		RETURN_ON_FAIL(UTurnOrderWidgetLog, Row);

		const bool bIsCurrent = (Row->GetUnitId() == CurrentUnitCombatId);
		Row->SetIsCurrent(bIsCurrent);
	}
}