// // Fill out your copyright notice in the Description page of Project Settings.
//
// #pragma once
//
// #include "CoreMinimal.h"
// #include "UObject/Object.h"
// #include "TurnMachine.generated.h"
//
// class ADefaultPlayerController;
// class ABaseUnitActor;
// class AWarpGameState;
// /**
//  * 
//  */
// UENUM(BlueprintType)
// enum class ETurnPhase : uint8 { AwaitInput, Moving };
//
// USTRUCT(BlueprintType)
// struct FTurnSnapshot
// {
// 	GENERATED_BODY()
//
// 	UPROPERTY()
// 	ETurnPhase Phase = ETurnPhase::AwaitInput;
// 	UPROPERTY()
// 	TObjectPtr<ABaseUnitActor> ActiveUnit = nullptr;
// 	UPROPERTY()
// 	int32 ActiveIndex = 0;
// };
//
// UCLASS()
// class WARP_API UTurnMachine : public UObject
// {
// 	GENERATED_BODY()
//
// public:
// 	void Init(AWarpGameState* InGS);
// 	
// 	void ServerInitUnits(const TArray<ABaseUnitActor*>& InUnits);
// 	bool ServerRequestMove(ADefaultPlayerController* PC, const FVector& Dest);
// 	void ServerOnUnitArrived(ABaseUnitActor* Unit);
//
// private:
// 	UPROPERTY()
// 	TObjectPtr<AWarpGameState> GS;
//
// 	void ServerSetActiveIndex(int32 Index);
// 	void ServerAdvance();
// 	bool ServerCanAcceptMove(ADefaultPlayerController* PC) const;
// };
