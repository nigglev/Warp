// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CombatUIWidget.generated.h"

class UBorder;
class UHorizontalBox;
class UButton;
/**
 * 
 */
UCLASS()
class WARP_API UCombatUIWidget : public UUserWidget
{
	GENERATED_BODY()

	
public:
	virtual void NativeConstruct() override;
	
	void ShowCombatUI(bool InShowCombatUI);
	void SetActionPoints(int32 CurrentPoints, int32 MaxPoints);

private:
	void RebuildActionPoints(int32 MaxPoints);
	void UpdateActionPointFill(int32 CurrentPoints);

	UFUNCTION()
	void HandleStartClicked();
	UFUNCTION()
	void HandleNextTurnClicked();
	
	UPROPERTY(meta=(BindWidget))
	UButton* StartButton = nullptr;
	UPROPERTY(meta=(BindWidget))
	UButton* NextTurnButton = nullptr;

	UPROPERTY(meta=(BindWidget))
	UHorizontalBox* ActionPointsBox = nullptr;
	UPROPERTY(EditAnywhere, Category="ActionPoints")
	FLinearColor FilledColor = FLinearColor::Green;
	UPROPERTY(EditAnywhere, Category="ActionPoints")
	FLinearColor EmptyColor = FLinearColor(0.f, 0.f, 0.f, 0.4f);
	UPROPERTY(EditAnywhere, Category="ActionPoints")
	int32 DefaultMaxPoints = 10;
	UPROPERTY()
	TArray<UBorder*> ActionPointWidgets;

};
