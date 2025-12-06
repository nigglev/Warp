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

class ADefaultGameMode;
class ADefaultWarpHUD;
class AUnitGhost;
class UTurnBasedSystemManager;
enum class ETurnPhase : uint8;
class AWarpGameState;
class UUnitBase;
class ACombatMapManager;
class ABaseUnitActor;
class FUnitGhostDeleter
{
	
};

class ADefaultPlayerController;
DECLARE_MULTICAST_DELEGATE_OneParam(FOnClientPlayerControllerValid, ADefaultPlayerController*);

/**
 * 
 */
UCLASS()
class WARP_API ADefaultPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ADefaultPlayerController();
	
	UFUNCTION(Server, Reliable)
	void ServerStartCombat();
	UFUNCTION(Server, Reliable)
	void ServerEndTurn();
	//SETUP//
	virtual void PostInitializeComponents() override;

	bool IsClientValidState() const { return CombatMapManager != nullptr; }

	FOnClientPlayerControllerValid OnDefaultPlayerControllerValid;

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;

	void SetupClientContent();
	void SetupEnhancedInput() const;
	void CreateCombatMapManager();
	
	void UpdateTileHovering();
	void UpdateUnitGhostPosition() const;

	void HandleEvents();
	void MoveCameraToUnit(uint32 InUnitID) const;
	//GET//
	ADefaultGameMode* GetGameMode() const;
	AWarpGameState* GetGameState() const;
	ADefaultWarpHUD* GetWarpHUD() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;
	bool GetHoveredTileIndexAndCoordinates(int32& OutInstanceIndex, FIntVector2& OutCoord) const;
	bool GetHoveredTileIndex(int32& OutInstanceIndex) const;
	uint32 GetMouseoverUnitID() const;

	//PLACEMENT//
	UFUNCTION()
	bool MoveUnitServerAuthoritative(const uint32 InUnitToMoveID, const FIntVector2& InGridPosition);
	UFUNCTION(Server, Reliable)
	void ServerRequestMoveUnit(const uint32 InUnitToMoveID, const FIntVector2& InGridPosition);
	UFUNCTION(Client, Reliable)
	void ClientPlacementResult(bool bSuccess);

	//INPUT ACTIONS//
	UFUNCTION()
	void OnRotateUnitGhostAction(const FInputActionValue& Value);
	UFUNCTION()
	void OnMoveUnitAction();
	UFUNCTION()
	void OnCameraMove(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraRotate(const FInputActionValue& Value);
	UFUNCTION()
	void OnCameraZoom(const FInputActionValue& Value);
	UFUNCTION()
	void OnRotateCameraPressed(const FInputActionValue& Value);
	UFUNCTION()
	void OnRotateCameraReleased(const FInputActionValue& Value);
	
	
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
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraMoveAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraRotateAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* StartCameraRotateAction = nullptr; 
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraZoomAction = nullptr;

	UFUNCTION()
	void HandleCombatStarted();
	UFUNCTION()
	void HandleActiveUnitChanged(uint32 InActiveUnitID);
	UFUNCTION()
	void HandleUnitsReadyLocal();
	
	UFUNCTION(Server, Reliable)
	void ServerSetContentReady();

	void CheckValidState();


	UPROPERTY()
	ABaseUnitActor* SelectedUnit = nullptr;
	UPROPERTY()
	ABaseUnitActor* GhostActor_ = nullptr;

	uint32 SelectedUnitID = 0;

	FIntVector2 HoveredTile = FIntVector2(-2, -2);
	FIntVector2 PrevHoveredTile = FIntVector2(-1, -1);
	uint32 LastFocusedTurnUnitId = 0;
	float MouseYawScaleDegPerUnit = 1.0f;

	bool bRotateCamera = false;
	bool bCameraLockedToTurnUnit = false;
	
	bool bPlacingUnit_ = false;
	bool bMovingUnit_ = false;

	bool bClientValidState_ = false; 
};




