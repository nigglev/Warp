// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "StartBattleWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class WARP_API UStartBattleWidget : public UUserWidget
{
	GENERATED_BODY()

	
public:
	virtual void NativeConstruct() override;

	UFUNCTION(BlueprintCallable)
	void SetEnabled(bool bEnabled);

private:
	UPROPERTY(meta=(BindWidget))
	UButton* StartButton = nullptr;

	UFUNCTION()
	void HandleStartClicked();
};
