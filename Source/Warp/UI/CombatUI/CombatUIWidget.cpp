// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatUIWidget.h"

#include "MGLogs.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"
#include "Warp/UI/UnitOrderUI/TurnOrderUnitInfo.h"
#include "Warp/UI/UnitOrderUI/TurnOrderWidget.h"
DEFINE_LOG_CATEGORY_STATIC(UCombatUIWidgetLog, Log, All);

void UCombatUIWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UCombatUIWidget::HandleStartClicked);
	}

	if (NextTurnButton)
	{	
		NextTurnButton->OnClicked.AddDynamic(this, &UCombatUIWidget::HandleNextTurnClicked);
		NextTurnButton->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCombatUIWidget::Init(ADefaultWarpHUD* InHUD)
{
	RETURN_ON_FAIL(UCombatUIWidgetLog, InHUD);
	HUD_ = InHUD;

	SubscribeToTurnBasedEvents();

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->Init(this);
		TurnOrderWidget_->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UCombatUIWidget::SubscribeToTurnBasedEvents()
{
	HUD_->GetTurnBasedSystemManager()->OnTurnOrderUpdated.AddUObject(
		this, &UCombatUIWidget::HandleTurnOrderUpdated);

	HUD_->GetTurnBasedSystemManager()->OnActiveUnitChanged.AddUObject(
		this, &UCombatUIWidget::HandleActiveUnitChanged);
}

void UCombatUIWidget::HandleTurnOrderUpdated(const TArray<uint32>& InTurnOrderUnitCombatIds, uint32 InCurrentTurnUnitCombatId)
{
	TurnOrderUnitCombatIds_ = InTurnOrderUnitCombatIds;
	CurrentTurnUnitCombatId_ = InCurrentTurnUnitCombatId;

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->RebuildFromHUD();
	}
}

void UCombatUIWidget::HandleActiveUnitChanged(uint32 InCurrentTurnUnitCombatId)
{
	CurrentTurnUnitCombatId_ = InCurrentTurnUnitCombatId;

	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->UpdateCurrentFromHUD();
	}
}

void UCombatUIWidget::HandleStartClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (auto* MyPC = Cast<ADefaultPlayerController>(PC))
		{
			MyPC->ServerStartCombat();
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
			MyPC->ServerEndTurn();
		}
	}
}

void UCombatUIWidget::ShowCombatUI(bool InShowCombatUI)
{
	if (StartButton)
	{
		StartButton->SetVisibility(ESlateVisibility::Collapsed);
	}
	if (NextTurnButton)
	{
		NextTurnButton->SetVisibility(ESlateVisibility::Visible);
	}
	if (TurnOrderWidget_)
	{
		TurnOrderWidget_->SetVisibility(ESlateVisibility::Visible);
	}
}


void UCombatUIWidget::GetTurnOrderInfo(TArray<uint32>& OutUnitCombatIds, uint32& OutCurrentUnitCombatId) const
{
	OutUnitCombatIds = TurnOrderUnitCombatIds_;
	OutCurrentUnitCombatId = CurrentTurnUnitCombatId_;
}

void UCombatUIWidget::GetTurnOrderUnitInfo(uint32 InUnitCombatId, FTurnOrderUnitInfo& OutInfo) const
{
	RETURN_ON_FAIL(UCombatUIWidgetLog, HUD_);
	
	OutInfo.UnitTypeName_ = HUD_->GetGameState()->GetUnitByID(InUnitCombatId)->GetUnitTypeName();
	EUnitAffiliation Affiliation = HUD_->GetGameState()->GetUnitByID(InUnitCombatId)->GetUnitAffiliation();
	if (Affiliation == EUnitAffiliation::Ally || Affiliation == EUnitAffiliation::Player)
		OutInfo.bIsAlly_ = true;
	if (Affiliation == EUnitAffiliation::Enemy)
		OutInfo.bIsAlly_ = false;
}
