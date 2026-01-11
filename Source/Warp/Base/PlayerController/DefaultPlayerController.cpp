// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

#include "HexGridWorldSubsystem.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "WarpCheatManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/Base/GameMode/DefaultGameMode.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"
#include "Warp/UI/CombatUI/CombatUIWidget.h"


DEFINE_LOG_CATEGORY_STATIC(ADefaultPlayerControllerLog, Log, All);

ADefaultPlayerController::ADefaultPlayerController()
{
	bReplicates = true;
	bShowMouseCursor = true;
	
	CheatClass = UWarpCheatManager::StaticClass();
}

void ADefaultPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (!IsLocalController())
		return;

	MG_FUNC_LABEL(ADefaultPlayerControllerLog);
	
#if !UE_BUILD_SHIPPING	
	EnableCheats();
#endif	
	
	UWarpPlayfabContentSubSystem* Content = UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL(ADefaultPlayerControllerLog, Content != nullptr);

	if (!Content->IsClientDataLoaded())
	{
		Content->OnUnitsLoaded.AddUObject(this, &ADefaultPlayerController::CheckClientLoading);
	}
}

void ADefaultPlayerController::OnMatchStateChanged(const FName& InMatchState)
{
	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("InMatchState: %s"), *InMatchState.ToString());
}

void ADefaultPlayerController::BeginPlay()
{
	MG_FUNC_LABEL(ADefaultPlayerControllerLog);
	
	Super::BeginPlay();

	SetupEnhancedInput();
}

void ADefaultPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (!IsClientLoaded())
		return;
}

void ADefaultPlayerController::OnRep_PlayerState()
{
	MG_FUNC_LABEL(ADefaultPlayerControllerLog);
	
	Super::OnRep_PlayerState();
	
	CheckClientLoading();
}

void ADefaultPlayerController::CheckClientLoading()
{
	RETURN_ON_FAIL(ADefaultPlayerControllerLog, IsLocalController());
	
	if (!IsClientLoaded())
		return;

	AWarpPlayerState* PS = GetPlayerState<AWarpPlayerState>();
	RETURN_ON_FAIL(ADefaultPlayerControllerLog, PS);
	PS->SetClientLoaded();

	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::PlayerController), TEXT("Valid State"));
	OnDefaultPlayerControllerValid.Broadcast(this);
}

bool ADefaultPlayerController::IsClientLoaded() const
{
	RETURN_ON_FAIL_BOOL(ADefaultPlayerControllerLog, IsLocalController());
	
	AWarpPlayerState* PS = GetPlayerState<AWarpPlayerState>();
	if (PS == nullptr)
	{
		return false;
	}

	UWarpPlayfabContentSubSystem* Content = UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL_BOOL(ADefaultPlayerControllerLog, Content != nullptr);

	return Content->IsClientDataLoaded();
}


void ADefaultPlayerController::SetupEnhancedInput() const
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsys =
			LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsys->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ADefaultPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(InputComponent))
	{
		EIC->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraMove);
		EIC->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraZoom);
		
		EIC->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraRotate);
		EIC->BindAction(StartCameraRotateAction, ETriggerEvent::Started,   this, &ADefaultPlayerController::OnRotateCameraPressed);
		
		EIC->BindAction(StartCameraRotateAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnRotateCameraReleased);
		EIC->BindAction(StartCameraRotateAction, ETriggerEvent::Canceled,  this, &ADefaultPlayerController::OnRotateCameraReleased);

		EIC->BindAction(Action_SelectCell, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCellAction<ECellType::Selected>);
		EIC->BindAction(Action_CaptureCell, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCellAction<ECellType::Captured>);
		EIC->BindAction(Action_CloseCell, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCellAction<ECellType::Closed>);
	}
}

void ADefaultPlayerController::OnCameraMove(const FInputActionValue& Value)
{
	const FVector2D Axis = Value.Get<FVector2D>();
	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		Cam->MoveXY(Axis, GetWorld() ? GetWorld()->GetDeltaSeconds() : 0.f);
	}
}

void ADefaultPlayerController::OnCameraRotate(const FInputActionValue& Value)
{
	if (!bRotateCamera) return;

	const float MouseX = Value.Get<float>();
	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		Cam->AddRotation(MouseX * MouseYawScaleDegPerUnit);
	}
}

void ADefaultPlayerController::OnRotateCameraPressed(const FInputActionValue& Value)
{
	bRotateCamera = true;
}

void ADefaultPlayerController::OnRotateCameraReleased(const FInputActionValue& Value)
{
	bRotateCamera = false;
}


void ADefaultPlayerController::OnCameraZoom(const FInputActionValue& Value)
{
	const float Axis = Value.Get<float>();
	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		Cam->AddZoom(Axis);
	}
}

template<ECellType InCellType>
void ADefaultPlayerController::OnCellAction(const FInputActionValue& Value)
{
	FVector P;
	if (GetMouseRayPlaneZIntersection(0.0f, P))
	{
		if (UWorld* World = GetWorld())
		{
			MG_LOG(ADefaultPlayerControllerLog,  TEXT("Coordinates: %s"), *P.ToString());
			DrawDebugSphere(World, P, 12.f, 16, FColor::Green, false, 1.0f);
			DrawDebugLine(World, P, P + FVector(0, 0, 50.f), FColor::Green, false, 1.0f, 0, 1.5f);
			
			UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
			if (GridWorldSubsystem != nullptr)
			{
				GridWorldSubsystem->SelectCell(P, InCellType);
			}
		}
	}
}

bool ADefaultPlayerController::GetMouseRayPlaneZIntersection(float PlaneZ, FVector& OutPoint) const
{
	FVector RayOrigin, RayDir;
	if (!DeprojectMousePositionToWorld(RayOrigin, RayDir))
	{
		return false;
	}

	if (FMath::Abs(RayDir.Z) < KINDA_SMALL_NUMBER)
	{
		return false;
	}

	const float T = (PlaneZ - RayOrigin.Z) / RayDir.Z;
	if (T < 0.0f)
	{
		return false;
	}

	OutPoint = RayOrigin + T * RayDir;
	return true;
}

ADefaultGameMode* ADefaultPlayerController::GetGameMode() const
{
	AGameModeBase* GM = GetWorld()->GetAuthGameMode();
	RETURN_ON_FAIL_NULL(ADefaultPlayerControllerLog, GM);
	
	if (ADefaultGameMode* GameMode = Cast<ADefaultGameMode>(GM))
	{
		return GameMode;
	}
	
	MG_COND_ERROR(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("GameMode is Invalid"));
	
	return nullptr;
}

AWarpGameState* ADefaultPlayerController::GetGameState() const
{
	if (AWarpGameState* GameState = GetWorld()->GetGameState<AWarpGameState>())
	{
		return GameState;
	}
	MG_COND_ERROR(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("GameState is Invalid"));
	return nullptr;
}

ADefaultWarpHUD* ADefaultPlayerController::GetWarpHUD() const
{
	if (ADefaultWarpHUD* WarpHUD = Cast<ADefaultWarpHUD>(MyHUD))
	{
		return WarpHUD;
	}
	MG_COND_ERROR(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("WarpHUD is Invalid"));
	return nullptr;
}

UTurnBasedSystemManager* ADefaultPlayerController::GetTurnBasedSystemManager() const
{
	if (UTurnBasedSystemManager* TBSM = GetGameState()->GetTurnBasedSystemManager())
	{
		return TBSM;
	}
	MG_COND_ERROR(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("TurnBasedSystemManager is Invalid"));
	return nullptr;
}

