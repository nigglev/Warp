// // Fill out your copyright notice in the Description page of Project Settings.
//
//
// #include "TurnMachine.h"
//
// #include "Warp/Base/GameState/WarpGameState.h"
//
// void UTurnMachine::Init(AWarpGameState* InGS)
// {
// 	GS = InGS;
// }
//
// void UTurnMachine::ServerInitUnits(const TArray<ABaseUnitActor*>& InUnits)
// {
// 	check(GS && GS->HasAuthority());
//
// 	GS->Units.Reset();
// 	for (ABaseUnitActor* U : InUnits)
// 	{
// 		GS->Units.Add(U);
// 		U->OnUnitArrived.AddUObject(this, &UTurnMachine::ServerOnUnitArrived);
// 	}
//
// 	ServerSetActiveIndex(0);
// }
//
// bool UTurnMachine::ServerRequestMove(ADefaultPlayerController* PC, const FVector& Dest)
// {
// 	if (!GS || !GS->HasAuthority()) return false;
// 	if (!ServerCanAcceptMove(PC)) return false;
// 	if (!GS->Turn.ActiveUnit) return false;
//
// 	FTurnSnapshot NewTurn = GS->Turn;
// 	NewTurn.Phase = ETurnPhase::Moving;
// 	GS->ServerSetTurnSnapshot(NewTurn);
//
// 	GS->Turn.ActiveUnit->ServerStartMove(Dest);
// 	return true;
// }
//
// void UTurnMachine::ServerOnUnitArrived(ABaseUnitActor* Unit)
// {
// 	if (!GS || !GS->HasAuthority()) return;
// 	if (Unit != GS->Turn.ActiveUnit) return; // ignore non-active arrivals
//
// 	FTurnSnapshot NewTurn = GS->Turn;
// 	NewTurn.Phase = ETurnPhase::AwaitInput;
// 	GS->ServerSetTurnSnapshot(NewTurn);
//
// 	ServerAdvance();
// }
//
// void UTurnMachine::ServerAdvance()
// {
// 	const int32 Num = GS->Units.Num();
// 	if (Num <= 0) return;
//
// 	int32 Next = GS->Turn.ActiveIndex + 1;
// 	ServerSetActiveIndex(Next);
// }
//
// void UTurnMachine::ServerSetActiveIndex(int32 Index)
// {
// 	const int32 Num = GS->Units.Num();
// 	if (Num <= 0) return;
//
// 	Index = (Index % Num + Num) % Num;
//
// 	FTurnSnapshot NewTurn;
// 	NewTurn.ActiveIndex = Index;
// 	NewTurn.ActiveUnit = GS->Units[Index];
// 	NewTurn.Phase = ETurnPhase::AwaitInput;
//
// 	GS->ServerSetTurnSnapshot(NewTurn);
// }
//
// bool UTurnMachine::ServerCanAcceptMove(ADefaultPlayerController* PC) const
// {
// 	if (!GS || !PC) return false;
// 	if (GS->Turn.Phase != ETurnPhase::AwaitInput) return false;
// 	
// 	return true;
// }