// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatUIWidget.h"

#include "MGLogs.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "TurnOrderUI/TurnOrderWidget.h"
#include "Warp/Base/GameMode/DefaultGameMode.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"

DEFINE_LOG_CATEGORY_STATIC(ACombatUIWidgetLog, Log, All);

void UCombatUIWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (NextTurnButton)
	{	
		NextTurnButton->OnClicked.AddDynamic(this, &UCombatUIWidget::HandleNextTurnClicked);
		NextTurnButton->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (ReturnToCampaignMapButton)
	{
		ReturnToCampaignMapButton->OnClicked.AddDynamic(this, &UCombatUIWidget::HandleReturnToCampaignMapClicked);
		
		ADefaultGameMode* GM = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());
		MG_COND_ERROR_SHORT(ACombatUIWidgetLog, GM == nullptr);
		if (GM)
		{
			ReturnToCampaignMapButton->SetVisibility(GM->GetMapNode() == EMapNodeType::Undefined 
				? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		}
	}
	
	ShowCombatUI(true);
}

void UCombatUIWidget::HandleNextTurnClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (auto* MyPC = Cast<ADefaultPlayerController>(PC))
		{
			MyPC->ActiveUnitStartMove();
		}
	}
}

void UCombatUIWidget::HandleReturnToCampaignMapClicked()
{
	ADefaultGameMode* GM = Cast<ADefaultGameMode>(GetWorld()->GetAuthGameMode());
	RETURN_ON_FAIL(ACombatUIWidgetLog, GM != nullptr);
	GM->ReturnToCampaignMap();
}

void UCombatUIWidget::ShowCombatUI(bool InShowCombatUI)
{	
	if (NextTurnButton)
	{
		NextTurnButton->SetVisibility(InShowCombatUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->SetVisibility(InShowCombatUI ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UCombatUIWidget::SetCurrentCombatUnits(const TArray<ABaseUnitActor*>& InCombatUnits, const int32 InActiveUnitIndex)
{
	RETURN_ON_FAIL(ACombatUIWidgetLog, TurnOrderWidget_);
	TurnOrderWidget_->RebuildFromHUD(InCombatUnits, InActiveUnitIndex);
}

void UCombatUIWidget::SetCurrentActiveUnitIndex(const int32 InActiveUnitIndex)
{
	RETURN_ON_FAIL(ACombatUIWidgetLog, TurnOrderWidget_);
	TurnOrderWidget_->UpdateCurrentFromHUD(InActiveUnitIndex);

}


void UCombatUIWidget::OnUnitSelected(ABaseUnitActor* InNewActiveUnit, ABaseUnitActor* InPrevActiveUnit)
{
	ShowCombatUI(true);
}

void UCombatUIWidget::OnUnitStartMoving(ABaseUnitActor* InNewActiveUnit)
{
	ShowCombatUI(false);
}

