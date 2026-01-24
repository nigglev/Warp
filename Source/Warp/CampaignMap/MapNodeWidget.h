// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignEnums.h"
#include "Blueprint/UserWidget.h"
#include "MapNodeWidget.generated.h"

class UMapViewportWidget;
/**
 * 
 */
UCLASS()
class WARP_API UMapNodeWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void Init(UMapViewportWidget* InOwner, FNodePosition InNodePosition, EMapNodeType InType, EMapNodeState InState);
	
	FNodePosition GetNodePosition() const { return NodePosition_; }
	void DropSelection();

	DECLARE_MULTICAST_DELEGATE_OneParam(FOnNodeClicked, UMapNodeWidget*);
	FOnNodeClicked OnNodeClicked;
	
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(EditAnywhere, Category="MapViewport") FLinearColor CompletedStateColor = FLinearColor::Gray;
	UPROPERTY(EditAnywhere, Category="MapViewport") FLinearColor UnaccessibleStateColor = FLinearColor::Red;
	UPROPERTY(EditAnywhere, Category="MapViewport") FLinearColor CapturedStateColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, Category="MapViewport") FLinearColor AvailableStateColor = FLinearColor::White;

	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UButton> Button_Node;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UImage>  Image_Icon;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UImage>  Image_Repair;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UImage>  Image_Shop;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UBorder> Border_Selected;
	UPROPERTY(meta=(BindWidget)) TObjectPtr<class UTextBlock> Text_Debug;

	void ApplyVisuals() const;
	UFUNCTION() void HandleClicked();
	
	void SetNodeColor(FLinearColor InColor) const;
	
	UPROPERTY()
	UMapViewportWidget* Owner_ = nullptr;
	
	FNodePosition NodePosition_; 
	
	EMapNodeType Type_ = EMapNodeType::Combat;
	EMapNodeState State_ = EMapNodeState::Available;
	bool bSelected_ = false;
};
