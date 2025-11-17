// Fill out your copyright notice in the Description page of Project Settings.


#include "StartBattleWidget.h"

#include "Components/Button.h"
#include "Warp/Base/PlayerController/DefaultPlayerController.h"

void UStartBattleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UStartBattleWidget::HandleStartClicked);
	}
}

void UStartBattleWidget::HandleStartClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (auto* MyPC = Cast<ADefaultPlayerController>(PC))
		{
			MyPC->ServerStartCombat();
		}
	}
	
	SetEnabled(false);
}

void UStartBattleWidget::SetEnabled(bool bEnabled)
{
	if (StartButton)
	{
		StartButton->SetIsEnabled(bEnabled);
	}
	SetVisibility(bEnabled ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}