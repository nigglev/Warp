// Fill out your copyright notice in the Description page of Project Settings.


#include "MapViewportWidget.h"

#include "MapNodeWidget.h"
#include "MGLogs.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"


DEFINE_LOG_CATEGORY_STATIC(AMapViewportWidgetLog, Log, All);

void UMapViewportWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RETURN_ON_FAIL(AMapViewportWidgetLog, InputCatcher);
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapBorder);
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapContentRoot);
	
	InputCatcher->OnMouseButtonDownEvent.BindUFunction(this,
		GET_FUNCTION_NAME_CHECKED(UMapViewportWidget, OnCatcherMouseDown));

	InputCatcher->OnMouseMoveEvent.BindUFunction(this,
		GET_FUNCTION_NAME_CHECKED(UMapViewportWidget, OnCatcherMouseMove));

	InputCatcher->OnMouseButtonUpEvent.BindUFunction(this,
		GET_FUNCTION_NAME_CHECKED(UMapViewportWidget, OnCatcherMouseUp));
	
	SpawnNodes();
}

void UMapViewportWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	CurrentOffset_ = FMath::Vector2DInterpTo(CurrentOffset_, TargetOffset_, InDeltaTime, InterpSpeed_);
	MapContentRoot->SetRenderTranslation(CurrentOffset_);
}

FEventReply UMapViewportWidget::OnCatcherMouseDown(FGeometry Geo, const FPointerEvent& E)
{
	if (E.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}

	
	TSharedPtr<SWidget> pInputCatcherWidget = InputCatcher->GetCachedWidget();
	if (!pInputCatcherWidget.IsValid())
		return UWidgetBlueprintLibrary::Unhandled();
	
	bMouseDown_=true; 
	PressPos_ = Geo.AbsoluteToLocal(E.GetScreenSpacePosition());
	LastPos_ = PressPos_;
	TargetOffset_ = CurrentOffset_;
	
	MG_LOG(AMapViewportWidgetLog, TEXT("PressPos: %s"), *PressPos_.ToString());
	
	FEventReply Reply(true);
	Reply.NativeReply = FReply::Handled().CaptureMouse(InputCatcher->TakeWidget());
	return Reply;	
}

FEventReply UMapViewportWidget::OnCatcherMouseMove(FGeometry Geo, const FPointerEvent& E)
{
	if (!bMouseDown_)
	{
		return UWidgetBlueprintLibrary::Unhandled();
	}
	
	FVector2f LocalPos = Geo.AbsoluteToLocal(E.GetScreenSpacePosition());
	FVector2f DraggingDelta = LocalPos - PressPos_;
	
	if (!bDragging_ && DraggingDelta.SquaredLength() > FMath::Square(DragThreshold_))
	{
		bDragging_ = true;
	}
	
	if (bDragging_)
	{
		FVector2f Delta = LocalPos - LastPos_;
		TargetOffset_ += FVector2D(Delta);

		TargetOffset_.Y = 0;
		
		const FGeometry& BorderGeo = MapBorder->GetCachedGeometry();
		FVector2f ViewSize = BorderGeo.GetLocalSize();
		float MaxX = FMath::Max(0, MaxX_ - ViewSize.X);
		TargetOffset_.X = FMath::Clamp(TargetOffset_.X, -MaxX, 0);
	}
	
	LastPos_ = LocalPos;
	
	return UWidgetBlueprintLibrary::Handled();
}

FEventReply UMapViewportWidget::OnCatcherMouseUp(FGeometry Geo, const FPointerEvent& E)
{
	bMouseDown_ = false;
	bDragging_ = false;

	FEventReply Reply(true);
	Reply.NativeReply = FReply::Handled().ReleaseMouseCapture();
	return Reply;
}

void UMapViewportWidget::SpawnNodes()
{
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapNodeClass);

	MapContentRoot->ClearChildren();
	SpawnedNodes_.Reset();
	
	float LayerWidthHS = LayerWidth_ / 2;
	
	{
		double XCenter = LayerWidthHS;
		double YCenter = LayerHeight_ / 2;
		FVector2D Pos(XCenter,YCenter);
		SpawnNode(FNodePosition(0, 0), Pos);
	}
	
	for (uint8 ILayer = 1; ILayer < LayerCount_; ++ILayer)
	{
		int32 NodeCount = RandomStream_.RandRange(NodeInLayerCountMin_, NodeInLayerCountMax_);
		
		float LayerHeightPadded = LayerHeight_ / NodeCount;
		float LayerHeightHS = LayerHeightPadded / 2;
		
		for (uint8 Step = 0; Step < NodeCount; ++Step)
		{
			double XCenter = LayerShift_ * ILayer + LayerWidthHS;
			double X = RandomStream_.RandRange(XCenter - LayerWidthHS * XDispersion_, XCenter + LayerWidthHS * XDispersion_);
			
			double YCenter = LayerVertPadding_ + LayerHeightPadded * Step + LayerHeightHS;
			double Y = RandomStream_.RandRange(YCenter - LayerHeightHS * YDispersion_, YCenter + LayerHeightHS * YDispersion_);
			
			FVector2D Pos(X,Y);
			SpawnNode(FNodePosition(ILayer, Step), Pos);	
		}
	}
	
	double LastXCenter = LayerShift_ * LayerCount_ + LayerWidthHS;
	{
		double YCenter = LayerHeight_ / 2;
		FVector2D Pos(LastXCenter,YCenter);
		SpawnNode(FNodePosition(LayerCount_, 0), Pos);
	}
	
	MaxX_ = LastXCenter + LayerWidthHS;
}

void UMapViewportWidget::SpawnNode(FNodePosition InNodePosition, const FVector2D& InPos)
{
	MG_LOG(AMapViewportWidgetLog, TEXT("InNodePosition: %s; %s"), *InNodePosition.ToString(), *InPos.ToString());
			
	UMapNodeWidget* Node = CreateWidget<UMapNodeWidget>(GetWorld(), MapNodeClass);
	RETURN_ON_FAIL(AMapViewportWidgetLog, Node);

	UCanvasPanelSlot* ChildSlot = MapContentRoot->AddChildToCanvas(Node);
	RETURN_ON_FAIL(AMapViewportWidgetLog, ChildSlot);

	ChildSlot->SetPosition(InPos);
	//ChildSlot->SetSize(NodeSize_);
	ChildSlot->SetAlignment(NodeAlign_);
	ChildSlot->SetZOrder(10);
	
	TArray<float> NodeTypeRanges({0.3f, 0.7f});
	
	float R = RandomStream_.GetFraction();
	
	int32 Index = Algo::UpperBound(NodeTypeRanges, R);
	
	EMapNodeType NodeType = static_cast<EMapNodeType>(Index);
	
	Node->Init(this, InNodePosition, NodeType, EMapNodeState::Available);

	SpawnedNodes_.Add(Node);
}

TValueOrError<bool, FString> UMapViewportWidget::TryToSelect(FNodePosition InNodePosition)
{
	if (InNodePosition == SelectedNodePosition_)
	{
		SelectedNodePosition_ = UnselectedNodePosition;
		return MakeValue(false);
	}
	
	if (SelectedNodePosition_ != UnselectedNodePosition)
	{
		int32 NodeIndex = SpawnedNodes_.IndexOfByPredicate([SelectedNodePosition=SelectedNodePosition_](const UMapNodeWidget* InNode) 
			{ return InNode->GetNodePosition() == SelectedNodePosition;});
		
		if (NodeIndex == INDEX_NONE)
		{
			return MakeError(TEXT("Node not found"));
		}
	
		SpawnedNodes_[NodeIndex]->DropSelection();
	}
	SelectedNodePosition_ = InNodePosition;
	
	return MakeValue(true);
}
