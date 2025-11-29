// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatUIWidget.h"

#include "Blueprint/WidgetTree.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"

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