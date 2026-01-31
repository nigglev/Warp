// Fill out your copyright notice in the Description page of Project Settings.


#include "ShipIconWidget.h"

#include "MapViewportWidget.h"
#include "MGLogs.h"
#include "Components/CanvasPanelSlot.h"

DEFINE_LOG_CATEGORY_STATIC(AShipIconWidgetLog, Log, All);

void UShipIconWidget::Init(UMapViewportWidget* InOwner)
{
	Owner_ = InOwner;
}

void UShipIconWidget::StartMove(FVector2D InPosition, FNodePosition InNodePosition)
{
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	RETURN_ON_FAIL(AShipIconWidgetLog, CanvasSlot);
	
	NodePosition_ = InNodePosition;
	
	StartPosition_ = CanvasSlot->GetPosition();
	TargetPosition_ = InPosition;
	
	StartAngle_ = GetRenderTransformAngle();
	
	const FVector2D D = (TargetPosition_ - StartPosition_);	
	TargetAngle_ = FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X));
		
	Elapsed_ = 0;
	
	bMoving_ = false;
	bRotation_ = true;
}

void UShipIconWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (bRotation_)
	{
		Elapsed_ += InDeltaTime;
		float Beta = FMath::Clamp(Elapsed_ / RotateDuration_, 0.f, 1.f);
		float Angle = FMath::InterpEaseInOut(StartAngle_, TargetAngle_, Beta, 1.f);
		SetRenderTransformAngle(Angle);
		if (Elapsed_ >= RotateDuration_)
		{
			Elapsed_ = 0;
			bRotation_ = false;
			bMoving_ = true;
		}
	}
	else if (bMoving_)
	{
		Elapsed_ += InDeltaTime;

		float Alpha = FMath::Clamp(Elapsed_ / MoveDuration_, 0.f, 1.f);
		FVector2D Pos = FMath::InterpEaseInOut(StartPosition_, TargetPosition_, Alpha, 1.f);
	
		UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
		RETURN_ON_FAIL(AShipIconWidgetLog, CanvasSlot);
	
		CanvasSlot->SetPosition(Pos);

		if (Elapsed_ >= MoveDuration_)
		{
			bMoving_ = false;

			MG_COND_ERROR_SHORT(AShipIconWidgetLog, Owner_ == nullptr);
			if (Owner_ != nullptr)
				Owner_->OnCaptureNode(NodePosition_);
		}
	}
}
