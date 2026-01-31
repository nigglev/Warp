// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CampaignEnums.h"
#include "Blueprint/UserWidget.h"
#include "MapViewportWidget.generated.h"

class UButton;
class UShipIconWidget;
class UImage;
class UMapNodeWidget;
class UCanvasPanel;
class UBorder;

/**
 * 
 */
UCLASS()
class WARP_API UMapViewportWidget : public UUserWidget
{
	GENERATED_BODY()

public:	
	TValueOrError<bool, FString> TryToSelect(FNodePosition InNodePosition);
	
	void OnCaptureNode(FNodePosition InNodePosition);
	
protected:
	virtual void NativeConstruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION() FEventReply OnCatcherMouseDown(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()	FEventReply OnCatcherMouseMove(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	UFUNCTION()	FEventReply OnCatcherMouseUp(FGeometry MyGeometry, const FPointerEvent& MouseEvent);
	
	void SpawnNodes();
	UMapNodeWidget* SpawnNode(FNodePosition InNodePosition, const FVector2D& InPos);	
	
	EMapNodeState GetNodeState(FNodePosition InNodePosition) const;

	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> InputCatcher;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UBorder> MapBorder;
	UPROPERTY(meta = (BindWidget)) TObjectPtr<UCanvasPanel> MapContentRoot;
	
	UPROPERTY(EditAnywhere, Category="MapViewport") float DragThreshold_ = 10;
	UPROPERTY(EditAnywhere, Category="MapViewport") float InterpSpeed_ = 10;
	
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") TSubclassOf<UMapNodeWidget> MapNodeClass;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") FVector2D NodeSize_ = FVector2D(64.0, 64.0);
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") FVector2D NodeAlign_ = FVector2D(0.5, 0.5);
	
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") int32 LayerCount_ = 3;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") int32 NodeInLayerCountMin_ = 3;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") int32 NodeInLayerCountMax_ = 4;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") float LayerWidth_ = 120;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") float LayerShift_ = 360;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") float XDispersion_ = 0.8f;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") float LayerHeight_ = 400;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") float LayerVertPadding_ = 100;
	UPROPERTY(EditAnywhere, Category="MapViewport|Nodes") float YDispersion_ = 0.6f;
	
	UPROPERTY(EditAnywhere, Category="MapViewport|Edges") float EdgeThickness_ = 3.0f;
	UPROPERTY(EditAnywhere, Category="MapViewport|Edges") bool bEdgeAntialias_ = true;
	UPROPERTY(EditAnywhere, Category="MapViewport|Edges") FLinearColor EdgeColor_ = FLinearColor(0.8f,0.9f,1.0f,0.35f);
	UPROPERTY(EditAnywhere, Category="MapViewport|Edges") TSubclassOf<UImage> LineSegmentClass;
	
	UPROPERTY(EditAnywhere, Category="MapViewport|Edges") TSubclassOf<UShipIconWidget> ShipIconWidgetClass;
	UPROPERTY() TObjectPtr<UShipIconWidget> ShipIconWidget_;	
	
	void BuildEdges();
	void GenerateEdges();
	void CreateShipIcon();

	void SpawnEdgeSegments(const FVector2D& A, const FVector2D& B, float Thickness);
	UImage* SpawnEdgeSegment(const FVector2D& A, const FVector2D& B, float Thickness);
	
	void Capture(FNodePosition InNodePosition);
	
	UFUNCTION()
	void DepartHandleClicked();
	
	void OnSelectNode(bool bSelect);
	
	void DropSelection();

	float MaxX_ = 0;
	
	bool bMouseDown_ = false;
	bool bDragging_ = false;
	
	bool bShipFlying = false;
	
	FVector2f PressPos_ = FVector2f::ZeroVector;
	FVector2f LastPos_ = FVector2f::ZeroVector;
	FVector2D TargetOffset_ = FVector2D::ZeroVector;
	FVector2D CurrentOffset_ = FVector2D::ZeroVector;
	
	FRandomStream RandomStream_;
	
	inline static const FNodePosition UnselectedNodePosition = FNodePosition(TNumericLimits<uint8>::Max(), TNumericLimits<uint8>::Max());
	
	FNodePosition SelectedNodePosition_ = UnselectedNodePosition;
	
	FNodePosition CapturedNodePosition_ = {0, 0};
	
	struct FNodeData
	{
		UMapNodeWidget* Node = nullptr;
		FVector2D Position;
				
		TArray<FNodePosition> Next_;

		FNodeData() = default;
		FNodeData(UMapNodeWidget* InNode, FVector2D InPosition) 
			: Node(InNode), Position(InPosition) {}
	};
	
	TArray<int32> NodeCountsInLayer_;
	
	TMap<FNodePosition, FNodeData> Nodes_;
	
	UPROPERTY()
	TArray<UImage*> Lines_;
	
	TWeakObjectPtr<UButton> DepartButton_;
};
