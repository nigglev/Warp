// Fill out your copyright notice in the Description page of Project Settings.


#include "MapNodeWidget.h"

#include "Components/TextBlock.h"

void UMapNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	ApplyVisuals();
}

void UMapNodeWidget::Init(uint8 InLayer, uint8 InStep)
{
	Layer_ = InLayer;
	InStep_ = InStep;
	
	ApplyVisuals();
}

void UMapNodeWidget::Setup(EMapNodeType InType, EMapNodeState InState, bool bInSelected, const FText& InDebug)
{
}

void UMapNodeWidget::ApplyVisuals()
{
	Text_Debug->SetText(FText::FromString(FString::Printf(TEXT("%d-%d"), Layer_, InStep_)));
}

void UMapNodeWidget::HandleClicked()
{
}
