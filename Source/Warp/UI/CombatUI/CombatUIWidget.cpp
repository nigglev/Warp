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

	if (ActionPointsBox)
	{
		ActionPointsBox->SetVisibility(ESlateVisibility::Collapsed);
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
	
	// if (TurnOrderWidgetClass_)
	// {
	// 	TurnOrderWidget_ = CreateWidget<UTurnOrderWidget>(this, TurnOrderWidgetClass_);
	// 	if (TurnOrderWidget_)
	// 	{
	// 		TurnOrderWidget_->Init(this);
	// 		TurnOrderWidget_->AddToViewport();
	// 		TurnOrderWidget_->SetVisibility(ESlateVisibility::Collapsed);
	// 	}
	// }
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
	if (ActionPointsBox)
	{
	    ActionPointsBox->SetVisibility(ESlateVisibility::Visible);
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


void UCombatUIWidget::SetActionPoints(int32 CurrentPoints, int32 MaxPoints)
{
	if (!ActionPointsBox)
	{
		return;
	}

	MaxPoints = FMath::Max(0, MaxPoints);
	CurrentPoints = FMath::Clamp(CurrentPoints, 0, MaxPoints);
	
	if (MaxPoints != ActionPointWidgets.Num())
	{
		RebuildActionPoints(MaxPoints);
	}

	UpdateActionPointFill(CurrentPoints);
}


void UCombatUIWidget::RebuildActionPoints(int32 MaxPoints)
{
	if (!ActionPointsBox || !WidgetTree)
	{
		return;
	}

	ActionPointWidgets.Reset();
	ActionPointsBox->ClearChildren();

	if (MaxPoints <= 0)
	{
		return;
	}

	const float SquareSize = 20.f;

	for (int32 i = 0; i < MaxPoints; ++i)
	{
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		if (!SizeBox)
		{
			continue;
		}

		SizeBox->SetWidthOverride(SquareSize);
		SizeBox->SetHeightOverride(SquareSize);
		
		UBorder* Border = WidgetTree->ConstructWidget<UBorder>(UBorder::StaticClass());
		if (!Border)
		{
			continue;
		}
		
		FSlateBrush Brush;
		Brush.DrawAs = ESlateBrushDrawType::Box;
		Border->SetBrush(Brush);
		Border->SetBrushColor(EmptyColor);
		Border->SetPadding(FMargin(0.f));
		
		SizeBox->SetContent(Border);
		
		if (UHorizontalBoxSlot* HorizontalBoxSlot = ActionPointsBox->AddChildToHorizontalBox(SizeBox))
		{
			HorizontalBoxSlot->SetPadding(FMargin(2.f, 0.f));
			HorizontalBoxSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
		}
		
		ActionPointWidgets.Add(Border);
	}
}

void UCombatUIWidget::UpdateActionPointFill(int32 CurrentPoints)
{
	for (int32 i = 0; i < ActionPointWidgets.Num(); ++i)
	{
		if (UBorder* Border = ActionPointWidgets[i])
		{
			const bool bFilled = (i < CurrentPoints);
			Border->SetBrushColor(bFilled ? FilledColor : EmptyColor);
		}
	}
}