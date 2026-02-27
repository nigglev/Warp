// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

#include "HexGridWorldSubsystem.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "WarpCheatManager.h"
#include "Warp/Actors/PlacePointer.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Actors/UnitActors/UnitActorFactory.h"
#include "Warp/ContentManagement/PlayFabContent/WarpPlayfabContentSubSystem.h"
#include "Warp/Base/GameMode/DefaultGameMode.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/Base/PlayerState/WarpPlayerState.h"
#include "Warp/ContentManagement/StaticDescriptions/WarpUnitDescriptions.h"
#include "Warp/TurnBasedSystem/TurnMachine.h"
#include "Warp/UI/HUD/DefaultWarpHUD.h"


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

	if (!Content->IsContentLoaded())
	{
		Content->OnContentLoaded.AddUObject(this, &ADefaultPlayerController::CheckClientLoading);
	}
	else
		CheckClientLoading();
}

void ADefaultPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADefaultPlayerController, ControlledUnit_);
}

void ADefaultPlayerController::BeginPlay()
{
	MG_FUNC_LABEL(ADefaultPlayerControllerLog);
	
	Super::BeginPlay();

	SetupEnhancedInput();

	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	SetInputMode(Mode);
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
	MG_FUNC_LABEL(ADefaultPlayerControllerLog);
	
	if (!IsClientLoaded())
	{
		MG_LOG(ADefaultPlayerControllerLog, TEXT("Client isn't Loaded"));
		return;
	}
	
	MG_LOG(ADefaultPlayerControllerLog, TEXT("Client is Loaded"));

	AWarpPlayerState* PS = GetPlayerState<AWarpPlayerState>();
	RETURN_ON_FAIL(ADefaultPlayerControllerLog, PS);
	PS->SetClientLoaded();

	OnDefaultPlayerControllerValid.Broadcast(this);
}

bool ADefaultPlayerController::IsClientLoaded() const
{
	RETURN_ON_FAIL_BOOL(ADefaultPlayerControllerLog, IsLocalController());

	UWarpPlayfabContentSubSystem* Content = UWarpPlayfabContentSubSystem::Get(this);
	RETURN_ON_FAIL_BOOL(ADefaultPlayerControllerLog, Content != nullptr);

	return Content->IsContentLoaded();
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

		EIC->BindAction(Action_SelectCell, ETriggerEvent::Started, this, &ADefaultPlayerController::OnSelectCellStartAction);
		EIC->BindAction(Action_SelectCell, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnSelectCellStopAction);
		
		EIC->BindAction(Action_CaptureCell, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCellAction<ECellType::Captured>);
		EIC->BindAction(Action_CloseCell, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCellAction<ECellType::Closed>);
		EIC->BindAction(Action_OpenCell, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCellAction<ECellType::Opened>);
		
		EIC->BindAction(Action_ShowDebugHUD, ETriggerEvent::Triggered, this, &ADefaultPlayerController::ShowDebugHUD);
	}
}

void ADefaultPlayerController::ShowDebugHUD(const FInputActionValue& Value)
{
	ADefaultWarpHUD* HUD = GetWarpHUD();
	RETURN_ON_FAIL(ADefaultPlayerControllerLog, HUD != nullptr);
	
	HUD->ShowDebugHUD();
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

void ADefaultPlayerController::OnSelectCellStartAction(const FInputActionValue& Value)
{
	MG_LOG(ADefaultPlayerControllerLog,  TEXT("Value: %s"), *Value.ToString());
	
	FVector P;
	if (GetMouseRayPlaneZIntersection(0.0f, P))
	{
		ABaseUnitActor* Unit = GetActiveUnit();
		if (!IsValid(Unit))
			return;
		
		const FUnitDescription* UnitDescr = Unit->GetDescription();
		RETURN_ON_FAIL(ADefaultPlayerControllerLog, UnitDescr != nullptr);
		
		UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
		RETURN_ON_FAIL(ADefaultPlayerControllerLog, GridWorldSubsystem != nullptr);
		
		TOptional<HexMath::FAxialCoord> TargetAxialCoordOpt = UHexGridWorldSubsystem::WorldToAxialCellCoord(P);
		RETURN_ON_FAIL(ADefaultPlayerControllerLog, TargetAxialCoordOpt.IsSet());
		
		TArray<HexMath::FPathNode> Path;
		GridWorldSubsystem->FindPath(Unit->GetAxialCoord(), Unit->GetAxialAngle().R, TargetAxialCoordOpt.GetValue(), {}, 
			UnitDescr->MaxRoundDistance, Path, UnitDescr->MoveCost, UnitDescr->RotationCost, true);
				
		if (!Path.IsEmpty())
		{
			if (PlacePointer_ == nullptr)
			{
				PlacePointer_ = Cast<APlacePointer>(UnitActorFactory::CreateActor(this, PlacePointerClass_, TargetAxialCoordOpt.GetValue()));
				RETURN_ON_FAIL(ADefaultPlayerControllerLog, PlacePointer_);
			}
		
			PlacePointer_->Set(Path.Last(), Unit);
		}
		else if (PlacePointer_ != nullptr)
		{
			PlacePointer_->FixRotation();
		}
	}
}

void ADefaultPlayerController::OnSelectCellStopAction(const FInputActionValue& Value)
{
	MG_LOG(ADefaultPlayerControllerLog,  TEXT("Value: %s"), *Value.ToString());
	// if (PlacePointer_)
	// {
	// 	ServerOrderMove(PlacePointer_->GetPathNode().Coord, PlacePointer_->GetAxialAngle());
	// 	
	// 	PlacePointer_->Destroy();
	// 	PlacePointer_ = nullptr;
	// 	
	// 	UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
	// 	RETURN_ON_FAIL(ADefaultPlayerControllerLog, GridWorldSubsystem != nullptr);
	// 	
	// 	GridWorldSubsystem->DropPathSelections();
	// }
}

void ADefaultPlayerController::ActiveUnitStartMove()
{
	if (PlacePointer_ == nullptr)
		return;
	
	ServerOrderMove(PlacePointer_->GetPathNode().Coord, PlacePointer_->GetAxialAngle());
		
	PlacePointer_->Destroy();
	PlacePointer_ = nullptr;
		
	UHexGridWorldSubsystem* GridWorldSubsystem = UHexGridWorldSubsystem::Get(this);
	RETURN_ON_FAIL(ADefaultPlayerControllerLog, GridWorldSubsystem != nullptr);
		
	GridWorldSubsystem->DropPathSelections();
}

void ADefaultPlayerController::ServerOrderMove_Implementation(const FRepAxialCoord& InTarget, const FAxialAngle& InAxialAngle)
{
	if (UTurnMachine* TM = GetGameState()->GetTurnMachine())
	{
		TM->RequestMove(InTarget, InAxialAngle);
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
				GridWorldSubsystem->SetCellType(P, InCellType);
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

ABaseUnitActor* ADefaultPlayerController::GetActiveUnit() const
{
	AWarpGameState* GS = GetGameState();
	RETURN_ON_FAIL_NULL(ADefaultPlayerControllerLog, GS);
	
	UTurnMachine* TM = GS->GetTurnMachine();
	RETURN_ON_FAIL_NULL(ADefaultPlayerControllerLog, TM);
	
	ABaseUnitActor* Unit = TM->GetActiveUnit();
	return Unit;
}