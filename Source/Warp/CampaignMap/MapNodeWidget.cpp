// Fill out your copyright notice in the Description page of Project Settings.


#include "MapNodeWidget.h"

#include "MapViewportWidget.h"
#include "MGLogs.h"
#include "Utilities.h"
#include "Components/Border.h"
#include "Components/Button.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

DEFINE_LOG_CATEGORY_STATIC(MapNodeWidget, Log, All);

void UMapNodeWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	Button_Node->OnClicked.AddDynamic(this, &UMapNodeWidget::HandleClicked);
	
	ApplyVisuals();
}

void UMapNodeWidget::Init(UMapViewportWidget* InOwner, FNodePosition InNodePosition, EMapNodeType InType, EMapNodeState InState)
{
	MG_COND_ERROR_SHORT(MapNodeWidget, InOwner == nullptr);
	
	Owner_ = InOwner;
	NodePosition_ = InNodePosition;
	Type_ = InType;
	State_ = InState;
	
	ApplyVisuals();
}

void UMapNodeWidget::ApplyVisuals() const
{
	Image_Repair->SetVisibility(Type_ == EMapNodeType::Repair ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	Image_Shop->SetVisibility(Type_ == EMapNodeType::Shop ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	
	TStringBuilder<128> StringBuilder;
	StringBuilder.Appendf(TEXT("%s "), *NodePosition_.ToString());
	
	if (Type_ != EMapNodeType::Combat)
	{
		StringBuilder.Appendf(TEXT("%s "), *ToStringEnum(Type_));
	}
	
	if (State_ != EMapNodeState::Available)
	{
		StringBuilder.Appendf(TEXT("%s"), *ToStringEnum(State_));
		
		Button_Node->SetVisibility(ESlateVisibility::Hidden);
	}
	
	if (State_ == EMapNodeState::Completed)	{ SetNodeColor(CompletedStateColor); }
	if (State_ == EMapNodeState::Available)	{ SetNodeColor(AvailableStateColor); }
	if (State_ == EMapNodeState::Unaccessible)	{ SetNodeColor(UnaccessibleStateColor); }
	if (State_ == EMapNodeState::Captured)	{ SetNodeColor(CapturedStateColor); }
	
	Text_Debug->SetText(FText::FromString(StringBuilder.ToString()));
	Text_Debug->SetVisibility(ESlateVisibility::Hidden);
}

void UMapNodeWidget::HandleClicked()
{
	MG_LOG(MapNodeWidget, TEXT("Clicked: %s"), *NodePosition_.ToString());
	
	TValueOrError<bool, FString> Selected = Owner_->TryToSelect(NodePosition_);
	
	if (Selected.HasError())
	{
		MG_ERROR(MapNodeWidget, TEXT("%s"), *Selected.GetError());
		return;
	}
	
	bool bSelected = Selected.GetValue();
	Border_Selected->SetVisibility(bSelected ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	
	MG_LOG(MapNodeWidget, TEXT("bSelected: %d"), bSelected);
}

void UMapNodeWidget::SetNodeColor(FLinearColor InColor) const
{
	Image_Icon->SetColorAndOpacity(InColor);
	Image_Repair->SetColorAndOpacity(InColor);
	Image_Shop->SetColorAndOpacity(InColor);
}

void UMapNodeWidget::DropSelection()
{
	Border_Selected->SetVisibility(ESlateVisibility::Hidden);
	MG_LOG(MapNodeWidget, TEXT("%s"), *NodePosition_.ToString());
}
