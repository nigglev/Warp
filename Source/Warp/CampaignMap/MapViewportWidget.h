// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapViewportWidget.generated.h"

class UCanvasPanel;
class UBorder;

/**
 * 
 */
UCLASS()
class WARP_API UMapViewportWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION()
	FEventReply OnCatcherMouseDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	UFUNCTION()
	FEventReply OnCatcherMouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	UFUNCTION()
	FEventReply OnCatcherMouseUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent);

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> InputCatcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> MapBorder;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MapContentRoot;
	
	UPROPERTY(EditAnywhere, Category="MapViewport")
	float DragThreshold_ = 10;
	
	UPROPERTY(EditAnywhere, Category="MapViewport")
	float InterpSpeed_ = 10;

	bool bMouseDown_ = false;
	bool bDragging_ = false;
	
	FVector2f PressPos_ = FVector2f::ZeroVector;
	FVector2f LastPos_ = FVector2f::ZeroVector;
	FVector2D TargetOffset_ = FVector2D::ZeroVector;
	FVector2D CurrentOffset_ = FVector2D::ZeroVector;
};
