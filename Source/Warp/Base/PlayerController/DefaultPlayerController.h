// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Warp/Units/UnitBase.h"
#include "DefaultPlayerController.generated.h"

class UTurnBasedSystemManager;
enum class ETurnPhase : uint8;
class AWarpGameState;
class UUnitBase;
class ACombatMapManager;
class ABaseUnitActor;
/**
 * 
 */
UCLASS()
class WARP_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADefaultPlayerController();
	void EnterPlacementMode(const FUnitSize& InSize);
	void CancelPlacementMode();
	
protected:
	//SETUP//
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	void SetupEnhancedInput() const;
	void CreateCombatMapManager();
	void TryInitCombatMapManager();
	void SetupTurnBasedSystemManager();
	
	//GHOST AND TILE PREVIEW//
	void SpawnGhost();
	void DestroyGhost();
	void UpdateTileHovering();
	void UpdateUnitPlacementPreview() const;
	void UpdateGhostPosition(const FIntPoint& InTile) const;

	//CAMERA//
	void UpdateCamera() const;

	//GET//
	AWarpGameState* GetGameState() const;
	bool GetHoveredTileIndexAndCoordinates(int32& OutInstanceIndex, FIntPoint& OutCoord) const;
	bool GetHoveredTileIndex(int32& OutInstanceIndex) const;
	ABaseUnitActor* GetMouseoverUnit() const;

	//PLACEMENT//
	UFUNCTION()
	bool PlaceUnitServerAuthoritative(const FIntPoint& CenterGrid);
	UFUNCTION(Server, Reliable)
	void ServerRequestPlaceUnit(const FIntPoint& CenterGrid, FUnitRotation Rotation, FUnitSize Size);
	UFUNCTION(Client, Reliable)
	void ClientPlacementResult(bool bSuccess);
	void CheckIfTilesAreAvailable(const FIntPoint& InTile) const;

	//INPUT ACTIONS//
	UFUNCTION()
	void OnRotateUnitGhostAction(const FInputActionValue& Value);
	UFUNCTION()
	void OnStartTurnAction();
	UFUNCTION()
	void OnCameraMove(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraRotate(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraZoom(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraToggleLock();
	UFUNCTION()
	void OnRMBPressed(const FInputActionValue& Value);
	UFUNCTION()
	void OnRMBReleased(const FInputActionValue& Value);

	//TURN LOGIC//
	UFUNCTION(Server, Reliable)
	void ServerStartTurns(int32 Seed = 12345);
	UFUNCTION(Server, Reliable)
	void ServerSpendAP(uint32 UnitId, int32 Amount = 1);
	UFUNCTION()
	void SpendOneOnCurrent();
	UFUNCTION()
	void HandleTurnPhaseChange(ETurnPhase InTurnPhase);
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<ACombatMapManager> CombatMapManagerClass;
	UPROPERTY()
	ACombatMapManager* CombatMapManager = nullptr;

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* RotateUnitGhostAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* PlaceUnitAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* MoveUnitAction;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputAction* StartTurnAction;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraMoveAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraRotateAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* RMBHoldAction = nullptr; 
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraZoomAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraToggleLockAction = nullptr;

	UPROPERTY()
	ABaseUnitActor* SelectedUnit = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Projection")
	TSubclassOf<ABaseUnitActor> GhostActorClass;
	UPROPERTY()
	ABaseUnitActor* GhostActor = nullptr;

	FUnitRotation PendingRotation;
	FUnitSize PendingSize;
	
	FIntPoint HoveredTile = FIntPoint(-2, -2);
	FIntPoint PrevHoveredTile = FIntPoint(-1, -1);
	uint32 LastFocusedTurnUnitId = 0;
	float MouseYawScaleDegPerUnit = 1.0f;

	bool bRotateCamera = false;
	bool bCameraLockedToTurnUnit = false;
	
	FTimerHandle InitMapTimerHandle;

public:
	
protected:
	UFUNCTION()
	void OnPlaceUnitAction();
	UFUNCTION()
	void OnMoveUnitAction();
	
	bool bPlacingUnit_ = false;
	bool bMovingUnit_ = false;
};



