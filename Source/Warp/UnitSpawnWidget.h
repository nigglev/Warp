// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitSpawnWidget.generated.h"

class UButton;
class UTextBlock;
class UHorizontalBox;
class UCanvasPanel;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnSmall);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnMedium);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpawnLarge);

/**
 * 
 */
UCLASS()
class WARP_API UUnitSpawnWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FOnSpawnSmall  OnSpawnSmall;
	UPROPERTY()
	FOnSpawnMedium OnSpawnMedium;
	UPROPERTY()
	FOnSpawnLarge  OnSpawnLarge;

protected:
	virtual void NativeConstruct() override;
	virtual TSharedRef<SWidget> RebuildWidget() override;

	UButton* MakeButton(const FString& Label) const;
	
	UFUNCTION()
	void HandleSmallClicked();
	UFUNCTION()
	void HandleMediumClicked();
	UFUNCTION()
	void HandleLargeClicked();

	UPROPERTY()
	UCanvasPanel* RootCanvas = nullptr;
	UPROPERTY()
	UHorizontalBox* ButtonRow = nullptr;
	UPROPERTY()
	UButton* BtnSmall = nullptr;
	UPROPERTY()
	UButton* BtnMedium = nullptr;
	UPROPERTY()
	UButton* BtnLarge = nullptr;
};
