// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StartupParamsUtility.h"
#include "Components/ActorComponent.h"
#include "MGLogTypesActorComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class DEBUGUTILITY_API UMGLogTypesActorComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UMGLogTypesActorComponent();
	
	virtual void InitializeComponent() override;

protected:
	void ReadStartupParams() const;

	UPROPERTY(EditInstanceOnly, Category="MGLogTypes")
	bool ParamsFromCommandLine = false;

	UPROPERTY(EditAnywhere, Category = "MGLogTypes", meta = (Bitmask, BitmaskEnum = "/Script/MGDebugTools.EMGLogTypes"))
	uint32 LogTypes = 0;

	UPROPERTY(EditAnywhere, Category = "MGChartTypes", meta = (Bitmask, BitmaskEnum = "/Script/MGDebugTools.EMGChartTypes"))
	uint32 ChartTypes = 0;

	FStartupParamsUtility StartupParamsUtility_;
};
