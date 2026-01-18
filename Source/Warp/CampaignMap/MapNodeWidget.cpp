// Fill out your copyright notice in the Description page of Project Settings.


#include "MapNodeWidget.h"

void UMapNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UMapNodeWidget::Init(uint8 InLayer, uint8 InStep)
{
	Layer_ = InLayer;
	InStep_ = InStep;
}

void UMapNodeWidget::Setup(EMapNodeType InType, EMapNodeState InState, bool bInSelected, const FText& InDebug)
{
}

void UMapNodeWidget::ApplyVisuals()
{
}

void UMapNodeWidget::HandleClicked()
{
}
