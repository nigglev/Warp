// Fill out your copyright notice in the Description page of Project Settings.


#include "MGLogTypesActorComponent.h"
#include "MGLogTypes.h"

DEFINE_LOG_CATEGORY_STATIC(ALogTypesActorComponent, Log, All);

namespace EStartupParams
{
	static const FName MGLogFlags(TEXT("MGLogFlags"));
	static const FName MGChartFlags(TEXT("MGChartFlags"));
}


// Sets default values for this component's properties
UMGLogTypesActorComponent::UMGLogTypesActorComponent(): StartupParamsUtility_({ EStartupParams::MGLogFlags })
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bWantsInitializeComponent = true;
}

void UMGLogTypesActorComponent::InitializeComponent()
{
	Super::InitializeComponent();
	ReadStartupParams();
}


void UMGLogTypesActorComponent::ReadStartupParams() const
{
#if WITH_EDITOR 
	if(!ParamsFromCommandLine)
	{
		MGLogTypes::CurrentLogs = LogTypes;
		MGLogTypes::CurrentCharts = ChartTypes;
	}
	else
#endif		
	{
		const FString& MGLogFlags = StartupParamsUtility_.GetValueAsString(EStartupParams::MGLogFlags);
		MGLogTypes::SetLogsFromStringEnum<EMGLogTypes>(MGLogFlags, MGLogTypes::CurrentLogs);

		const FString& MGChartFlags = StartupParamsUtility_.GetValueAsString(EStartupParams::MGChartFlags);
		MGLogTypes::SetLogsFromStringEnum<EMGChartTypes>(MGChartFlags, MGLogTypes::CurrentCharts);
	}
}
