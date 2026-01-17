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

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UBorder> InputCatcher;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<class UBorder> MapBorder;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> MapContentRoot;
};
