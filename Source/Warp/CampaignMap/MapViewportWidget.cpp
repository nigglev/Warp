// Fill out your copyright notice in the Description page of Project Settings.


#include "MapViewportWidget.h"
#include "MGLogs.h"

DEFINE_LOG_CATEGORY_STATIC(AMapViewportWidgetLog, Log, All);

void UMapViewportWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	MG_COND_ERROR_SHORT(AMapViewportWidgetLog, !InputCatcher);
	MG_COND_ERROR_SHORT(AMapViewportWidgetLog, !MapBorder);
	MG_COND_ERROR_SHORT(AMapViewportWidgetLog, !MapContentRoot);
}
