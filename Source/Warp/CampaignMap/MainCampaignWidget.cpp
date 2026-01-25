// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCampaignWidget.h"

#include "MGLogs.h"
#include "Components/Button.h"

DEFINE_LOG_CATEGORY_STATIC(MainCampaignWidget, Log, All);

void UMainCampaignWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	MG_COND_ERROR_SHORT(MainCampaignWidget, Button_Depart == nullptr);
	if (Button_Depart != nullptr)
	{
		Button_Depart->SetVisibility(ESlateVisibility::Collapsed);
		Button_Depart->OnClicked.AddDynamic(this, &UMainCampaignWidget::HandleClicked);
	}
}

void UMainCampaignWidget::OnSelectNode(bool bSelect, FNodePosition IntNodePosition)
{
	RETURN_ON_FAIL(MainCampaignWidget, Button_Depart);
	Button_Depart->SetVisibility(bSelect ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UMainCampaignWidget::HandleClicked()
{
}
