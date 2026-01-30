// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShipIconWidget.generated.h"

/**
 * 
 */
UCLASS()
class WARP_API UShipIconWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:	
	void StartMove(FVector2D InPosition);
	
protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;	
	
	UPROPERTY(EditAnywhere, Category="Map") float MoveDuration_ = 3;
	UPROPERTY(EditAnywhere, Category="Map") float RotateDuration_ = 1;
	
private:
	
	bool bMoving_ = false;
	bool bRotation_ = false;
	
	float Elapsed_ = 0;
	
	FVector2D StartPosition_;
	FVector2D TargetPosition_;
	
	float StartAngle_ = 0;
	float TargetAngle_ = 0;
};
