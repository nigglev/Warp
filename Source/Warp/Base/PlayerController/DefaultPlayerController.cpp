// Fill out your copyright notice in the Description page of Project Settings.


#include "DefaultPlayerController.h"

#include "IContentBrowserSingleton.h"
#include "MGLogs.h"
#include "MGLogTypes.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "Warp/Actors/CombatMapManager/CombatMapManager.h"
#include "Warp/Actors/UnitActors/BaseUnitActor.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/Base/Pawn/TacticalCameraPawn.h"
#include "Warp/TurnBasedSystem/Manager/TurnBasedSystemManager.h"

DEFINE_LOG_CATEGORY_STATIC(ADefaultPlayerControllerLog, Log, All);

ADefaultPlayerController::ADefaultPlayerController()
{
	bReplicates = true;
	bShowMouseCursor = true;
}

void ADefaultPlayerController::BeginPlay()
{
	Super::BeginPlay();
	SetupEnhancedInput();
	CreateCombatMapManager();
	SetupTurnBasedSystemManager();
}

void ADefaultPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	UpdateTileHovering();
	UpdateUnitGhostPosition();
	UpdateCamera();
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
		EIC->BindAction(PlaceUnitAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnPlaceUnitAction);
		EIC->BindAction(StartTurnAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnStartTurnAction);
		EIC->BindAction(RotateUnitGhostAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnRotateUnitGhostAction);

		EIC->BindAction(CameraMoveAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraMove);
		EIC->BindAction(CameraRotateAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraRotate);
		EIC->BindAction(CameraZoomAction, ETriggerEvent::Triggered, this, &ADefaultPlayerController::OnCameraZoom);
		EIC->BindAction(CameraToggleLockAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnCameraToggleLock);

		EIC->BindAction(RMBHoldAction, ETriggerEvent::Started,   this, &ADefaultPlayerController::OnRMBPressed);
		EIC->BindAction(RMBHoldAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnRMBReleased);
		EIC->BindAction(RMBHoldAction, ETriggerEvent::Canceled,  this, &ADefaultPlayerController::OnRMBReleased);

		EIC->BindAction(MoveUnitAction, ETriggerEvent::Started,   this, &ADefaultPlayerController::OnMoveUnitAction);
		//EIC->BindAction(MoveUnitAction, ETriggerEvent::Completed, this, &ADefaultPlayerController::OnMoveUnitAction);
		//EIC->BindAction(MoveUnitAction, ETriggerEvent::Canceled,  this, &ADefaultPlayerController::OnMoveUnitAction);
	}
}


void ADefaultPlayerController::CreateCombatMapManager()
{
	MG_COND_ERROR_SHORT(ADefaultPlayerControllerLog, IsLocalController());
	MG_COND_ERROR_SHORT(ADefaultPlayerControllerLog, CombatMapManagerClass);

	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	CombatMapManager = GetWorld()->SpawnActor<ACombatMapManager>(
		CombatMapManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, Params);

	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Spawned local CombatMapManager [%s]"), *GetNameSafe(CombatMapManager));
	
	TryInitCombatMapManager();
	if (!InitMapTimerHandle.IsValid())
	{
		GetWorldTimerManager().SetTimer(InitMapTimerHandle, this,
			&ADefaultPlayerController::TryInitCombatMapManager, 0.1f, true);
	}
	
}

void ADefaultPlayerController::TryInitCombatMapManager()
{
	if (!CombatMapManager) return;

	if (AWarpGameState* GS = GetGameState())
	{
		const uint32 Grid = GS->GetMapGridSize();
		const uint32 Tile = GS->GetMapTileSize();

		if (Grid > 0 && Tile > 0)
		{
			CombatMapManager->Init(Grid, Tile);
			CombatMapManager->GenerateGrid();

			if (InitMapTimerHandle.IsValid())
			{
				GetWorldTimerManager().ClearTimer(InitMapTimerHandle);
			}

			MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
				TEXT("CombatMapManager initialized from GameState (Grid=%d, Tile=%d)"), Grid, Tile);
		}
	}
}

void ADefaultPlayerController::SetupTurnBasedSystemManager()
{
	if (AWarpGameState* GS = GetGameState())
	{
		if (UTurnBasedSystemManager* TM = GS->GetTurnBasedSystemManager())
		{
			TM->OnTurnPhaseChanged.AddUObject(this, &ADefaultPlayerController::HandleTurnPhaseChange);
		}
		else
		{
			FTimerHandle H; GetWorldTimerManager().SetTimer(H, [this]()
			{
				if (AWarpGameState* GS2 = GetGameState())
					if (UTurnBasedSystemManager* TM2 = GS2->GetTurnBasedSystemManager())
						TM2->OnTurnPhaseChanged.AddUObject(this, &ADefaultPlayerController::HandleTurnPhaseChange);
				}, 0.05f, false);
		}
	}
}

void ADefaultPlayerController::UpdateTileHovering()
{
	MG_COND_ERROR_SHORT(ADefaultPlayerControllerLog, CombatMapManager == nullptr);

	int32 TileIndex;
	PrevHoveredTile = HoveredTile;
	if (!GetHoveredTileIndexAndCoordinates(TileIndex, HoveredTile))
		return;
	if (TileIndex < 0 || HoveredTile.X < 0 || HoveredTile.Y < 0)
		return;
	if (HoveredTile == PrevHoveredTile)
		return;
	CombatMapManager->UpdateHoveredTile(TileIndex);
}

void ADefaultPlayerController::UpdateUnitGhostPosition() const
{
	if ((!bPlacingUnit_ && !bMovingUnit_) || !IsValid(GhostActor_))
		return;
	if (HoveredTile == PrevHoveredTile)
		return;
	const FVector WorldCenter = CombatMapManager->GridToLevelPosition(HoveredTile);
	CheckIfTilesAreAvailable(HoveredTile, GhostActor_->GetUnitActorSize(), GhostActor_->GetUnitActorRotation());
	GhostActor_->UpdatePosition(WorldCenter);
}

void ADefaultPlayerController::UpdateCamera() const
{
	if (bCameraLockedToTurnUnit)
	{
		if (AWarpGameState* GS = GetGameState())
		{
			if (UUnitBase* Current = GS->GetTurnBasedSystemManager()->GetCurrentTurnUnit())
			{
				if (CombatMapManager)
				{
					const FVector2f Pos2D = Current->UnitPosition.GetUnitWorldPosition();
					const FVector WorldPos = CombatMapManager->CombatMapToLevelPosition(Pos2D);

					if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
					{
						Cam->SetFollowTarget(WorldPos);
					}
				}
			}
		}
	}
}

void ADefaultPlayerController::OnRotateUnitGhostAction(const FInputActionValue& Value)
{
	if (!IsValid(GhostActor_))
		return;
	const float Delta = Value.Get<float>();
	if (Delta > 0)
		GhostActor_->Rotate(true);
	if (Delta < 0)
		GhostActor_->Rotate(false);
}

void ADefaultPlayerController::OnPlaceUnitAction()
{
	if (bPlacingUnit_)
	{
		PlaceUnitServerAuthoritative(HoveredTile);
	}	
}

void ADefaultPlayerController::OnMoveUnitAction()
{
	if (bPlacingUnit_)
		return;
	
	bMovingUnit_ = !bMovingUnit_;
	if (bMovingUnit_)
	{
		ABaseUnitActor* UnitActor = GetMouseoverUnit();
		if (!UnitActor)
			return;
		GhostActor_ = CombatMapManager->SpawnUnitActorGhost(UnitActor);
	}
	else
	{
		CombatMapManager->DestroyGhostActor(GhostActor_);
	}
	
}

void ADefaultPlayerController::OnStartTurnAction()
{
	if (GetGameState()->GetTurnBasedSystemManager()->GetCurrentTurnPhase() == ETurnPhase::None)
	{
		ServerStartTurns(12345);
		return;
	}
	SpendOneOnCurrent();
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


void ADefaultPlayerController::OnRMBPressed(const FInputActionValue& Value)
{
	bRotateCamera = true;
}

void ADefaultPlayerController::OnRMBReleased(const FInputActionValue& Value)
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

void ADefaultPlayerController::OnCameraToggleLock()
{
	bCameraLockedToTurnUnit = !bCameraLockedToTurnUnit;

	if (ATacticalCameraPawn* Cam = Cast<ATacticalCameraPawn>(GetPawn()))
	{
		if (bCameraLockedToTurnUnit)
		{
			if (AWarpGameState* GS = GetGameState())
			{
				if (UUnitBase* Current = GS->GetTurnBasedSystemManager()->GetCurrentTurnUnit(); Current && CombatMapManager)
				{
					const FVector2f Pos2D = Current->UnitPosition.GetUnitWorldPosition();
					const FVector WorldPos = CombatMapManager->CombatMapToLevelPosition(Pos2D);
					Cam->SetLockedToTarget(true, WorldPos);
				}
				else
				{
					Cam->SetLockedToTarget(true, GetPawn() ? GetPawn()->GetActorLocation() : FVector::ZeroVector);
				}
			}
		}
		else
		{
			Cam->SetLockedToTarget(false);
		}
	}
}


void ADefaultPlayerController::EnterPlacementMode(const FUnitSize& InSize)
{
	bPlacingUnit_ = true;
	GhostActor_ = CombatMapManager->SpawnUnitActorGhost(FVector2f::ZeroVector, InSize, FUnitRotation::Rot0());
}


void ADefaultPlayerController::CancelPlacementMode()
{
	bPlacingUnit_ = false;
	CombatMapManager->DestroyGhostActor(GhostActor_);
}

void ADefaultPlayerController::ServerStartTurns_Implementation(int32 Seed)
{
	if (AWarpGameState* GS = GetGameState())
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("Starting turns on server {%d} with seed = %d"), HasAuthority(), Seed);
		GS->GetTurnBasedSystemManager()->StartTurns(Seed);
	}
}


void ADefaultPlayerController::SpendOneOnCurrent()
{
	if (AWarpGameState* GS = GetGameState())
	{
		if (UUnitBase* U = GS->GetTurnBasedSystemManager()->GetCurrentTurnUnit())
		{
 			ServerSpendAP(U->GetUnitCombatID(), 1);
		}
	}
}

void ADefaultPlayerController::HandleTurnPhaseChange(ETurnPhase InTurnPhase)
{
	OnCameraToggleLock();
	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("TurnPhaseChange"));
}

void ADefaultPlayerController::ServerSpendAP_Implementation(uint32 UnitId, int32 Amount)
{
	AWarpGameState* GS = GetGameState();
	if (!IsValid(GS))
		return;

	UUnitBase* U = GS->FindUnitByCombatID(UnitId);
	if (!U)
		return;
	
	if (GS->GetTurnBasedSystemManager()->GetCurrentTurnUnit() != U)
		return;

	if (!U->SpendAP(Amount))
		return;
	
	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Reducing action point from unit id = %d by 1 {HasAuthority: %d}"),U->GetUnitCombatID(), HasAuthority());
	
	if (!U->HasAnyAffordableAction(GS->GetTurnBasedSystemManager()->GetMinActionPointCost()))
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("All points for unit id %d has been spent {HasAuthority: %d}"), U->GetUnitCombatID(), HasAuthority());
		GS->GetTurnBasedSystemManager()->AdvanceToNextUnit();
	}
}

bool ADefaultPlayerController::PlaceUnitServerAuthoritative(const FIntPoint& InGridPosition)
{
	if (!CombatMapManager) return false;
	
	TArray<FIntPoint> Blockers;
	FUnitSize UnitSize = GhostActor_->GetUnitActorSize();
	FUnitRotation UnitRotation = GhostActor_->GetUnitActorRotation();
	if (!CombatMapManager->IsPositionForUnitAvailable(InGridPosition, UnitRotation, UnitSize, Blockers))
		return false;

	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("Requesting to place unit with Rotation = %f, Size = %s at a pos %s"),
	UnitRotation.GetUnitFRotation(), *UnitSize.GetUnitTileLength().ToString(), *InGridPosition.ToString());
	
	ServerRequestPlaceUnit(InGridPosition, UnitRotation, UnitSize);
	
	return true;
}

void ADefaultPlayerController::ServerRequestPlaceUnit_Implementation(const FIntPoint& InGridPosition, FUnitRotation Rotation,
	FUnitSize Size)
{
	AWarpGameState* GS = GetGameState();
	if (!IsValid(GS))
	{
		ClientPlacementResult(false);
		return;
	}
	
	MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
	TEXT("Requesting to place unit with Rotation = %f, Size = %s at a pos %s"),
	Rotation.GetUnitFRotation(), *Size.GetUnitTileLength().ToString(), *InGridPosition.ToString());

	UUnitBase* NewUnit = nullptr;
	const bool bSuccess = GS->CreateUnitAt(0, InGridPosition, Rotation, NewUnit);
	ClientPlacementResult(bSuccess);
}

void ADefaultPlayerController::CheckIfTilesAreAvailable(const FIntPoint& InTile, const FUnitSize& InUnitSize, const FUnitRotation& InUnitRotation) const
{
	TArray<FIntPoint> OutBlockers;
	if (CombatMapManager->IsPositionForUnitAvailable(InTile, InUnitRotation, InUnitSize, OutBlockers))
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
			TEXT("GOOD POSITION"));
	}
	CombatMapManager->UpdateVisualForBlockers(OutBlockers);
}

void ADefaultPlayerController::ClientPlacementResult_Implementation(bool bSuccess)
{
	if (bSuccess)
	{
		CancelPlacementMode();
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Unit placed successfully"));
	}
	else
	{
		MG_COND_LOG(ADefaultPlayerControllerLog, MGLogTypes::IsLogAccessed(EMGLogTypes::DefaultPlayerController),
		TEXT("Failed to place unit"));
	}
}

ABaseUnitActor* ADefaultPlayerController::GetMouseoverUnit() const
{
	FHitResult Hit;
	if (GetHitResultUnderCursorByChannel(TraceTypeQuery1, true, Hit))
	{
		return Cast<ABaseUnitActor>(Hit.GetActor());
	}
	return nullptr;
}


bool ADefaultPlayerController::GetHoveredTileIndexAndCoordinates(int32& OutInstanceIndex, FIntPoint& OutCoord) const
{
	int32 HitIdx;
	if (GetHoveredTileIndex(HitIdx))
	{
		OutInstanceIndex = HitIdx;
		OutCoord = CombatMapManager->TileInstanceToGridPosition(HitIdx);
		return true;
	}
	return false;
}

bool ADefaultPlayerController::GetHoveredTileIndex(int32& OutInstanceIndex) const
{
	OutInstanceIndex = INDEX_NONE;

	FHitResult Hit;
	if (!GetHitResultUnderCursorByChannel(ETraceTypeQuery::TraceTypeQuery1, true, Hit))
	{
		return false;
	}
	
	if (CombatMapManager && Hit.Component == CombatMapManager->GetTilesISMC())
	{
		int32 TileIndex;
		if (Hit.Item >= CombatMapManager->GetGridSquare())
			TileIndex = Hit.Item - CombatMapManager->GetGridSquare();
		else
		{
			TileIndex = Hit.Item;
		}
		OutInstanceIndex = TileIndex;
		return true;
	}

	return false;
}

AWarpGameState* ADefaultPlayerController::GetGameState() const
{
	if (AWarpGameState* GameState = GetWorld()->GetGameState<AWarpGameState>())
	{
		return GameState;
	}
	return nullptr;
}

