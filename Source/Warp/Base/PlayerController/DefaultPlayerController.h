// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Warp/Units(Deprecated)/UnitBase.h"
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
	//SETUP//
	virtual void PostInitializeComponents() override;

	bool IsClientLoaded() const;

	FOnClientPlayerControllerValid OnDefaultPlayerControllerValid;
	
	void OnMatchStateChanged(const FName& InMatchState);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void PlayerTick(float DeltaTime) override;
	
	void CheckClientLoading();
	virtual void OnRep_PlayerState() override;

	void SetupEnhancedInput() const;
	//GET//
	ADefaultGameMode* GetGameMode() const;
	AWarpGameState* GetGameState() const;
	ADefaultWarpHUD* GetWarpHUD() const;
	UTurnBasedSystemManager* GetTurnBasedSystemManager() const;
	
	//INPUT ACTIONS//
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

	UPROPERTY(EditDefaultsOnly, Category="Input")
	UInputMappingContext* DefaultMappingContext;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraMoveAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraRotateAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* StartCameraRotateAction = nullptr; 
	UPROPERTY(EditDefaultsOnly, Category="Input|Camera")
	class UInputAction* CameraZoomAction = nullptr;
	UPROPERTY(EditDefaultsOnly, Category="Input")
	class UInputAction* LMBAction;
	
	
	
	void OnLMBStarted(const FInputActionValue& Value);
	bool GetMouseRayPlaneZIntersection(float PlaneZ, FVector& OutPoint) const;
	
	float MouseYawScaleDegPerUnit = 1.0f;
	bool bRotateCamera = false;
};




