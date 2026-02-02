#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "HexagonGridSettings.generated.h"

#define HEX_LAYOUT HexMath::EHexOffsetLayout::FlatTopOddQ

class AHexGridISMActor;
/**
 *
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Hexagon Grid Settings"))
class HEXAGONGRID_API UHexagonGridSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UHexagonGridSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static UHexagonGridSettings* Get() 
	{ 
		return GetMutableDefault<UHexagonGridSettings>();
	}
	// config - need for save

	UPROPERTY(EditAnywhere, Config, Category="HexGrid")
	TSubclassOf<AHexGridISMActor> HexGridActorClass_;
	
	UPROPERTY(EditAnywhere, Config, Category="HexGrid")
	int32 BuildChunkAround = 1;
	
	UPROPERTY(EditAnywhere, Config, Category="HexGrid")
	int32 SelectRadius = 1;
	
	UPROPERTY(EditAnywhere, Config, Category="HexGrid")
	bool PathfinderLog = false;
};
