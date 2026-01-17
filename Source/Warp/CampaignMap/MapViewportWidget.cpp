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
	
	SpawnTestNodes();
}

void UMapViewportWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	//const FGeometry& BorderGeo = MapBorder->GetCachedGeometry();
	//FVector2f ViewSize = BorderGeo.GetLocalSize();
	
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
		//TargetOffset_ = ClampOffset(TargetOffset_);
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

void UMapViewportWidget::SpawnTestNodes()
{
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapNodeClass);

	MapContentRoot->ClearChildren();
	SpawnedNodes_.Reset();

	const TArray<FVector2D> TestPositions = {
		{ 200, 200 },
		{ 400, 260 },
		{ 650, 180 },
		{ 800, 320 },
	};

	for (const FVector2D& Pos : TestPositions)
	{
		UMapNodeWidget* Node = CreateWidget<UMapNodeWidget>(GetWorld(), MapNodeClass);
		RETURN_ON_FAIL(AMapViewportWidgetLog, Node);

		UCanvasPanelSlot* ChildSlot = MapContentRoot->AddChildToCanvas(Node);
		RETURN_ON_FAIL(AMapViewportWidgetLog, ChildSlot);

		ChildSlot->SetPosition(Pos);
		//ChildSlot->SetSize(NodeSize_);
		ChildSlot->SetAlignment(NodeAlign_);
		ChildSlot->SetZOrder(10);

		SpawnedNodes_.Add(Node);
	}
}
