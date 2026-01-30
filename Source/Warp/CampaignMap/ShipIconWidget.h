// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignEnums.h"
#include "Blueprint/UserWidget.h"
#include "ShipIconWidget.generated.h"

class UMapViewportWidget;
/**
 * 
 */
UCLASS()
class WARP_API UShipIconWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Init(UMapViewportWidget* InOwner);
	void StartMove(FVector2D InPosition, FNodePosition InNodePosition);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;	
	
	UPROPERTY(EditAnywhere, Category="Map") float MoveDuration_ = 3;
	UPROPERTY(EditAnywhere, Category="Map") float RotateDuration_ = 1;
	
private:
	
	UPROPERTY()
	UMapViewportWidget* Owner_;
	
	FNodePosition NodePosition_;
	
	bool bMoving_ = false;
	bool bRotation_ = false;
	
	float Elapsed_ = 0;
	
	FVector2D StartPosition_;
	FVector2D TargetPosition_;
	
	float StartAngle_ = 0;
	float TargetAngle_ = 0;
};
