// Fill out your copyright notice in the Description page of Project Settings.


#include "MainMenuWidget.h"

#include "MainMenuButton.h"
#include "MGLogs.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
DEFINE_LOG_CATEGORY_STATIC(UMainMenuLog, Log, All);

bool UMainMenuWidget::Initialize()
{
	const bool bResult = Super::Initialize();
	if (!bResult)
	{
		return false;
	}

	RETURN_ON_FAIL_BOOL(UMainMenuLog, NewGameButton);
	RETURN_ON_FAIL_BOOL(UMainMenuLog, OptionsButton);
	RETURN_ON_FAIL_BOOL(UMainMenuLog, ExitButton);
	
	NewGameButton->SetText(FText::FromString(TEXT("New Game")));
	NewGameButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnNewGameClicked);

	OptionsButton->SetText(FText::FromString(TEXT("Options")));
	OptionsButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnOptionsClicked);

	ExitButton->SetText(FText::FromString(TEXT("Exit")));
	ExitButton->OnClicked.AddDynamic(this, &UMainMenuWidget::OnExitClicked);

	return true;
}

void UMainMenuWidget::OnNewGameClicked()
{
	UE_LOG(LogTemp, Log, TEXT("New Game clicked"));
	// Next step: open ship selection widget or menu state switch
}

void UMainMenuWidget::OnOptionsClicked()
{
	UE_LOG(LogTemp, Log, TEXT("Options clicked"));
	// Placeholder for now
}

void UMainMenuWidget::OnExitClicked()
{
	UKismetSystemLibrary::QuitGame(this, nullptr, EQuitPreference::Quit, false);
}