// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "CampaignHUD.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API ACampaignHUD : public AHUD
{
	GENERATED_BODY()

public:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UUserWidget> MainWidgetClass_;
	
	UPROPERTY(Transient)
	TObjectPtr<UUserWidget> MainWidget_;
};
