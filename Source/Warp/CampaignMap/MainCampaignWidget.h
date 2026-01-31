// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignEnums.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "MainCampaignWidget.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API UMainCampaignWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	TWeakObjectPtr<UButton> GetDepartButton() const { return Button_Depart.Get(); }
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta=(BindWidget)) TObjectPtr<UButton> Button_Depart;
};
