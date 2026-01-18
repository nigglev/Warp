// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignEnums.h"
#include "Blueprint/UserWidget.h"
#include "MapNodeWidget.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API UMapNodeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Init(uint8 InLayer, uint8 InStep);
	void Setup(EMapNodeType InType, EMapNodeState InState, bool bInSelected, const FText& InDebug);

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnNodeClicked, UMapNodeWidget*);
	FOnNodeClicked OnNodeClicked;
	
protected:
	virtual void NativeConstruct() override;

	//UPROPERTY(meta=(BindWidget)) TObjectPtr<class UButton> Button_ClickArea;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UImage>  Image_Icon;
	//UPROPERTY(meta=(BindWidget)) TObjectPtr<class UBorder> Border_StateFrame;
	//UPROPERTY(meta=(BindWidget)) TObjectPtr<class UBorder> Border_SelectedOverlay;
	//UPROPERTY(meta=(BindWidget)) TObjectPtr<class UTextBlock> Text_Debug;

	void ApplyVisuals();
	UFUNCTION() void HandleClicked();
	
	uint8 Layer_ = 0;
	uint8 InStep_ = 0;
	
	EMapNodeType Type_ = EMapNodeType::Combat;
	EMapNodeState State_ = EMapNodeState::Locked;
	bool bSelected_ = false;
};
