// Fill out your copyright notice in the Description page of Project Settings.


#include "MapViewportWidget.h"

#include "CampaignGameMode.h"
#include "CampaignHUD.h"
#include "MapNodeWidget.h"
#include "MGLogs.h"
#include "ShipIconWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpGameplayDescriptions.h"


DEFINE_LOG_CATEGORY_STATIC(AMapViewportWidgetLog, Log, All);

void UMapViewportWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	RETURN_ON_FAIL(AMapViewportWidgetLog, InputCatcher);
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapBorder);
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapContentRoot);

	UWarpPlayfabContentSubSystem* Content = UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(AMapViewportWidgetLog, Content);

	Content->OnContentLoaded.AddUObject(this, &UMapViewportWidget::BuildMap);
	
	InputCatcher->OnMouseButtonDownEvent.BindUFunction(this,
		GET_FUNCTION_NAME_CHECKED(UMapViewportWidget, OnCatcherMouseDown));

	InputCatcher->OnMouseMoveEvent.BindUFunction(this,
		GET_FUNCTION_NAME_CHECKED(UMapViewportWidget, OnCatcherMouseMove));

	InputCatcher->OnMouseButtonUpEvent.BindUFunction(this,
		GET_FUNCTION_NAME_CHECKED(UMapViewportWidget, OnCatcherMouseUp));
	
	ACampaignGameMode* GM = Cast<ACampaignGameMode>(GetWorld()->GetAuthGameMode());
	MG_COND_ERROR_SHORT(AMapViewportWidgetLog, GM == nullptr);
	if (GM != nullptr)
	{
		CapturedNodePosition_ = GM->GetNodePosition();
	}
	
	APlayerController* PC = GetOwningPlayer();
	RETURN_ON_FAIL(AMapViewportWidgetLog, PC != nullptr);
	
	ACampaignHUD* HUD = Cast<ACampaignHUD>(PC->GetHUD());
	RETURN_ON_FAIL(AMapViewportWidgetLog, HUD != nullptr);
	
	DepartButton_ = HUD->GetDepartButton();
	RETURN_ON_FAIL(AMapViewportWidgetLog, DepartButton_.IsValid());
	
	DepartButton_->OnClicked.AddDynamic(this, &UMapViewportWidget::DepartHandleClicked);
	
	OnSelectNode(false);
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

void UMapViewportWidget::BuildMap()
{
	const FGameplayDescription* GameplayDescriptions = GetGameplayDescriptions();
	RETURN_ON_FAIL(AMapViewportWidgetLog, GameplayDescriptions);
	
	LayerCount_ = GameplayDescriptions->CampaignMapLayerCount;
	NodeInLayerCountMin_ = GameplayDescriptions->CampaignMapNodeInLayerCountMin;
	NodeInLayerCountMax_ = GameplayDescriptions->CampaignMapNodeInLayerCountMax;
	
	SpawnNodes();
	BuildEdges();
	CreateShipIcon();
}

void UMapViewportWidget::SpawnNodes()
{
	RETURN_ON_FAIL(AMapViewportWidgetLog, MapNodeClass);

	//MapContentRoot->ClearChildren();
	Nodes_.Reset();
	
	float LayerWidthHS = LayerWidth_ / 2;
	
	{
		double XCenter = LayerWidthHS;
		double YCenter = LayerHeight_ / 2;
		FVector2D Pos(XCenter,YCenter);
		FNodePosition NodePos(0, 0);
		UMapNodeWidget* Node = SpawnNode(NodePos, Pos);
		Nodes_.Add(NodePos, FNodeData(Node, Pos));
		NodeCountsInLayer_.Add(1);
	}
	
	for (uint8 ILayer = 1; ILayer < LayerCount_; ++ILayer)
	{
		int32 NodeCount = RandomStream_.RandRange(NodeInLayerCountMin_, NodeInLayerCountMax_);
		NodeCountsInLayer_.Add(NodeCount);
		
		float LayerHeightPadded = LayerHeight_ / NodeCount;
		float LayerHeightHS = LayerHeightPadded / 2;
		
		for (uint8 Step = 0; Step < NodeCount; ++Step)
		{
			double XCenter = LayerShift_ * ILayer + LayerWidthHS;
			double X = RandomStream_.RandRange(XCenter - LayerWidthHS * XDispersion_, XCenter + LayerWidthHS * XDispersion_);
			
			double YCenter = LayerVertPadding_ + LayerHeightPadded * Step + LayerHeightHS;
			double Y = RandomStream_.RandRange(YCenter - LayerHeightHS * YDispersion_, YCenter + LayerHeightHS * YDispersion_);
			
			FVector2D Pos(X,Y);
			FNodePosition NodePos(ILayer, Step);
			UMapNodeWidget* Node = SpawnNode(NodePos, Pos);
			
			Nodes_.Add(NodePos, FNodeData(Node, Pos));
		}
	}
	
	double LastXCenter = LayerShift_ * LayerCount_ + LayerWidthHS;
	{
		double YCenter = LayerHeight_ / 2;
		FVector2D Pos(LastXCenter,YCenter);
		FNodePosition NodePos(LayerCount_, 0);
		UMapNodeWidget* Node = SpawnNode(NodePos, Pos);
		Nodes_.Add(NodePos, FNodeData(Node, Pos));
		NodeCountsInLayer_.Add(1);
	}
	
	MaxX_ = LastXCenter + LayerWidthHS;
}

UMapNodeWidget* UMapViewportWidget::SpawnNode(FNodePosition InNodePosition, const FVector2D& InPos)
{
	RETURN_ON_FAIL_NULL(AMapViewportWidgetLog, MapNodeClass);
	
	MG_LOG(AMapViewportWidgetLog, TEXT("InNodePosition: %s; %s"), *InNodePosition.ToString(), *InPos.ToString());
			
	UMapNodeWidget* Node = CreateWidget<UMapNodeWidget>(GetWorld(), MapNodeClass);
	RETURN_ON_FAIL_NULL(AMapViewportWidgetLog, Node);

	UCanvasPanelSlot* ChildSlot = MapContentRoot->AddChildToCanvas(Node);
	RETURN_ON_FAIL_NULL(AMapViewportWidgetLog, ChildSlot);

	ChildSlot->SetPosition(InPos);
	//ChildSlot->SetSize(NodeSize_);
	ChildSlot->SetAlignment(NodeAlign_);
	ChildSlot->SetZOrder(10);
	
	//ЖЕСТЬ под переделку с настройками
	TArray<float> NodeTypeRanges({0.3f, 0.7f});
	
	float R = RandomStream_.GetFraction();
	
	int32 Index = Algo::UpperBound(NodeTypeRanges, R);
	
	EMapNodeType NodeType = static_cast<EMapNodeType>(Index + 1);
	
	EMapNodeState NodeState = GetNodeState(InNodePosition); 
	
	Node->Init(this, InNodePosition, NodeType, NodeState);
	
	return Node;
}

EMapNodeState UMapViewportWidget::GetNodeState(FNodePosition InNodePosition) const
{
	if (InNodePosition == CapturedNodePosition_)
		return EMapNodeState::Captured;
	
	if (InNodePosition.X <= CapturedNodePosition_.X)
		return EMapNodeState::Completed;
	
	if (InNodePosition.X > CapturedNodePosition_.X + 1)
		return EMapNodeState::Unaccessible;
	
	return EMapNodeState::Available;
}

void UMapViewportWidget::BuildEdges()
{
	GenerateEdges();
	
	for (const auto& Edge : Nodes_)
	{
		const FVector2D A = Edge.Value.Position;
		for (auto Next : Edge.Value.Next_)
		{
			if (const FNodeData* NextNodeData = Nodes_.Find(Next))
			{
				SpawnEdgeSegments(A, NextNodeData->Position, EdgeThickness_);
			}
		}
	}
}

int32 FindClosestWeightIndex(const float W, const TArray<float>& Weights)
{
	if (Weights.IsEmpty())
	{
		return INDEX_NONE;
	}

	int32 BestIndex = INDEX_NONE;
	float BestDist = TNumericLimits<float>::Max();

	for (int32 i = 0; i < Weights.Num(); ++i)
	{
		const float V = Weights[i];
		if (!FMath::IsFinite(V))
		{
			continue; // пропускаем NaN/Inf
		}

		const float Dist = FMath::Abs(V - W);
		if (Dist < BestDist)
		{
			BestDist = Dist;
			BestIndex = i;
		}
	}

	return BestIndex;
}


void UMapViewportWidget::GenerateEdges()
{
	
	for (int32 Layer = 0; Layer < NodeCountsInLayer_.Num() - 1; ++Layer)
	{
		uint8 CurrentLayerNodeCount = NodeCountsInLayer_[Layer];
		uint8 NextLayerNodeCount = NodeCountsInLayer_[Layer + 1];
		
		static TArray<float> NextWeights;
		NextWeights.Reset();
		for (uint8 j = 0; j < NextLayerNodeCount; ++j)
		{
			NextWeights.Add((j + 1) / (float)NextLayerNodeCount);
		}
		
		static TArray<int32> UnusedIndexes;
		UnusedIndexes.Reset();
		for (uint8 j = 0; j < NextLayerNodeCount; ++j)
		{
			UnusedIndexes.Add(j);
		}
		
		static TArray<float> CurrentWeights;
		CurrentWeights.Reset();
		
		for (uint8 Y = 0; Y < CurrentLayerNodeCount; ++Y)
		{
			float CurrentWeight = (Y + 1) / (float)CurrentLayerNodeCount;
			CurrentWeights.Add(CurrentWeight);
			
			int32 ClosestIndex = FindClosestWeightIndex(CurrentWeight, NextWeights);
			
			FNodePosition CurrentNodePosition(Layer, Y);
			
			FNodeData* Node = Nodes_.Find(CurrentNodePosition);
			MG_COND_ERROR_SHORT(AMapViewportWidgetLog, Node == nullptr);
			if (Node != nullptr)
			{
				Node->Next_.Add(FNodePosition(Layer + 1, ClosestIndex));
				UnusedIndexes.Remove(ClosestIndex);
				
				if (ClosestIndex > 0 && RandomStream_.GetFraction() > 0.5f)
				{
					Node->Next_.Add(FNodePosition(Layer + 1, ClosestIndex - 1));
					UnusedIndexes.Remove(ClosestIndex - 1);
				}
				
				if (ClosestIndex < NextWeights.Num() - 1 && RandomStream_.GetFraction() > 0.5f)
				{
					Node->Next_.Add(FNodePosition(Layer + 1, ClosestIndex + 1));
					UnusedIndexes.Remove(ClosestIndex + 1);
				}
			}
		}
		
		for (int32 UnusedIndex : UnusedIndexes)
		{
			int32 ClosestIndex = FindClosestWeightIndex(NextWeights[UnusedIndex], CurrentWeights);
			
			FNodePosition CurrentNodePosition(Layer, ClosestIndex);
			
			FNodeData* Node = Nodes_.Find(CurrentNodePosition);
			MG_COND_ERROR_SHORT(AMapViewportWidgetLog, Node == nullptr);
			if (Node != nullptr)
			{
				FNodePosition UnusedNodePosition(Layer + 1, UnusedIndex);
				Node->Next_.Add(UnusedNodePosition);
			}
		}
		
//		uint8 NodeIndex = RandomStream_.RandRange(0, NodeCount);
	}
}

void UMapViewportWidget::SpawnEdgeSegments(const FVector2D& A, const FVector2D& B, float Thickness)
{
	FVector2D Delta = B - A;
	
	float Len = Delta.Length();
	if (Len < 0.001f) return;
	
	Delta /= Len;
	
	int32 N = 25;
	float SmallLen = Len / N;
	for (int32 I = 1; I < N; ++I)
	{
		if ((I & 0x1) == 0x1)
		{
			UImage* Img = SpawnEdgeSegment(A + Delta * SmallLen * I, A + Delta * SmallLen * (I + 1), Thickness);
			Lines_.Add(Img);
		}
	}
}

UImage* UMapViewportWidget::SpawnEdgeSegment(const FVector2D& A, const FVector2D& B, float Thickness)
{
	UImage* Img = NewObject<UImage>(this, LineSegmentClass);
	//Img->SetBrushFromTexture(nullptr); // или WhiteBrush в BP
	Img->SetColorAndOpacity(EdgeColor_); // FLinearColor с альфой
	Img->SetRenderTransformPivot(FVector2D(0.f, 0.5f)); // левый центр

	MapContentRoot->AddChild(Img);

	const FVector2D D = (B - A);
	const float Len = D.Size();
	const float AngleDeg = FMath::RadiansToDegrees(FMath::Atan2(D.Y, D.X));

	if (UCanvasPanelSlot* ChildSlot = Cast<UCanvasPanelSlot>(Img->Slot))
	{
		ChildSlot->SetAutoSize(false);
		ChildSlot->SetAlignment(FVector2D(0.f, 0.5f));     // позиция = левый центр
		ChildSlot->SetPosition(A);
		ChildSlot->SetSize(FVector2D(Len, Thickness));
		ChildSlot->SetZOrder(-10);
	}

	FWidgetTransform T;
	T.Angle = AngleDeg;
	Img->SetRenderTransform(T);

	return Img;
}

void UMapViewportWidget::CreateShipIcon()
{
	FNodeData* Node = Nodes_.Find(CapturedNodePosition_);
	RETURN_ON_FAIL(AMapViewportWidgetLog, Node);
	
	RETURN_ON_FAIL(AMapViewportWidgetLog, ShipIconWidgetClass);
	ShipIconWidget_ = CreateWidget<UShipIconWidget>(this, ShipIconWidgetClass);
	RETURN_ON_FAIL(AMapViewportWidgetLog, ShipIconWidget_);
	
	ShipIconWidget_->Init(this);
	
	ShipIconWidget_->SetRenderTransformPivot(FVector2D(0.5f, 0.5f));
	
	UCanvasPanelSlot* ChildSlot = MapContentRoot->AddChildToCanvas(ShipIconWidget_);
	RETURN_ON_FAIL(AMapViewportWidgetLog, ChildSlot);

	ChildSlot->SetPosition(Node->Position);
	ChildSlot->SetAlignment(NodeAlign_);
	ChildSlot->SetZOrder(20);
}

TValueOrError<bool, FString> UMapViewportWidget::TryToSelect(FNodePosition InNodePosition)
{
	if (bShipFlying)
	{
		return MakeValue(false);
	}
	
	if (InNodePosition == SelectedNodePosition_)
	{
		SelectedNodePosition_ = UnselectedNodePosition;
		OnSelectNode(false);
		return MakeValue(false);
	}
	
	FNodeData* CapNode = Nodes_.Find(CapturedNodePosition_);
	RETURN_ON_FAIL_DEFAULT(AMapViewportWidgetLog, CapNode != nullptr, MakeError(TEXT("Captured Node is null")));
	
	if (!CapNode->Next_.Contains(InNodePosition))
	{
		return MakeValue(false);
	}
	
	if (SelectedNodePosition_ != UnselectedNodePosition)
	{
		FNodeData* Node = Nodes_.Find(SelectedNodePosition_);
		if (Node == nullptr)
		{
			return MakeError(TEXT("Node not found"));
		}
		if (Node->Node == nullptr)
		{
			return MakeError(TEXT("Node Widget is null"));
		}
		Node->Node->DropSelection();
	}
	SelectedNodePosition_ = InNodePosition;
	OnSelectNode(true);
		
	return MakeValue(true);
}

void UMapViewportWidget::DropSelection()
{
	if (UnselectedNodePosition == SelectedNodePosition_)
		return;
	
	FNodeData* Node = Nodes_.Find(SelectedNodePosition_);
	RETURN_ON_FAIL(AMapViewportWidgetLog, Node != nullptr);
	
	Node->Node->DropSelection();
	
	SelectedNodePosition_ = UnselectedNodePosition;
	OnSelectNode(false);
}

const FGameplayDescription* UMapViewportWidget::GetGameplayDescriptions()
{
	//RETURN_ON_FAIL_NULL(AMapViewportWidgetLog, GetWorld());

	UWarpPlayfabContentSubSystem* Content = UWarpPlayfabContentSubSystem::Get(this);
	//RETURN_ON_FAIL(AMapViewportWidgetLog, Content);
	return Content->GetDescription<FGameplayDescription>(FName("GameplayDescriptions"));
}

void UMapViewportWidget::OnSelectNode(bool bSelect)
{
	RETURN_ON_FAIL(AMapViewportWidgetLog, DepartButton_.IsValid());
	DepartButton_->SetVisibility(bSelect ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UMapViewportWidget::DepartHandleClicked()
{
	RETURN_ON_FAIL(AMapViewportWidgetLog, ShipIconWidget_ != nullptr);
	RETURN_ON_FAIL(AMapViewportWidgetLog, SelectedNodePosition_ != UnselectedNodePosition);
	
	FNodeData* Node = Nodes_.Find(SelectedNodePosition_);
	RETURN_ON_FAIL(AMapViewportWidgetLog, Node);
	
	ShipIconWidget_->StartMove(Node->Position, SelectedNodePosition_);
	
	DropSelection();
	
	bShipFlying = true;
}

void UMapViewportWidget::OnCaptureNode(FNodePosition InNodePosition)
{
	bShipFlying = false;
	Capture(InNodePosition);
}

void UMapViewportWidget::Capture(FNodePosition InNodePosition)
{
	CapturedNodePosition_ = InNodePosition;

	for (TPair<FNodePosition, FNodeData> NodePair : Nodes_)
	{
		EMapNodeState NodeState = GetNodeState(NodePair.Key);
		NodePair.Value.Node->SetState(NodeState);
	}
	
	ACampaignGameMode* GM = Cast<ACampaignGameMode>(GetWorld()->GetAuthGameMode());
	RETURN_ON_FAIL(AMapViewportWidgetLog, GM);
	
	FNodeData* Node = Nodes_.Find(CapturedNodePosition_);
	RETURN_ON_FAIL(AMapViewportWidgetLog, Node);
	
	GM->OnCapture(CapturedNodePosition_, Node->Node->GetNodeType());
}