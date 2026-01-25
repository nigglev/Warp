// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignEnums.h"
#include "GameFramework/HUD.h"
#include "CampaignHUD.generated.h"

class UMainCampaignWidget;
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
	
	void OnSelectNode(bool bSelect, FNodePosition IntNodePosition);

private:
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UMainCampaignWidget> MainWidgetClass_;
	
	UPROPERTY(Transient)
	TObjectPtr<UMainCampaignWidget> MainWidget_;
};
