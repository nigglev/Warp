// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UMainMenuButton;
class UButton;
/**
 * 
 */
UCLASS()
class WARP_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual bool Initialize() override;

	UPROPERTY(meta = (BindWidget))
	UMainMenuButton* NewGameButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	UMainMenuButton* OptionsButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	UMainMenuButton* ExitButton = nullptr;

	UFUNCTION()
	void OnNewGameClicked();

	UFUNCTION()
	void OnOptionsClicked();

	UFUNCTION()
	void OnExitClicked();
};
