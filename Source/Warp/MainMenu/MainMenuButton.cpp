// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuButton.h"

#include "MGLogs.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(UMainMenuButtonLog, Log, All);

bool UMainMenuButton::Initialize()
{
	const bool bResult = Super::Initialize();
	if (!bResult)
	{
		return false;
	}
	RETURN_ON_FAIL_BOOL(UMainMenuButtonLog, MainMenuButton);
	RETURN_ON_FAIL_BOOL(UMainMenuButtonLog, MainMenuButtonText);

	MainMenuButton->OnClicked.AddDynamic(this, &UMainMenuButton::HandleClicked);
	return true;
}

void UMainMenuButton::SetText(const FText& InText)
{
	if (MainMenuButtonText)
	{
		MainMenuButtonText->SetText(InText);
	}
}

void UMainMenuButton::HandleClicked()
{
	OnClicked.Broadcast();
}


