// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuButton.generated.h"

class UTextBlock;
class UEditableTextBox;
class UButton;
/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FMainMenuButtonClicked);


UCLASS()
class WARP_API UMainMenuButton : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual bool Initialize() override;

	void SetText(const FText& InText);

	UPROPERTY(BlueprintAssignable)
	FMainMenuButtonClicked OnClicked;

protected:
	UFUNCTION()
	void HandleClicked();
	
	UPROPERTY(meta = (BindWidget))
	UButton* MainMenuButton = nullptr;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MainMenuButtonText = nullptr;
};
