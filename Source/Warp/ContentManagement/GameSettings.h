#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GameSettings.generated.h"

class ABaseUnitActor;
/**
 * 
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Warp Game Settings"))
class WARP_API UGameSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UGameSettings(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	static UGameSettings* Get() 
	{ 
		return GetMutableDefault<UGameSettings>();
	}
	
	TSubclassOf<ABaseUnitActor> GetUnitActorClass(const FName& InUnitType, bool InGhost = false) const;
	
	UPROPERTY(Config, EditAnywhere, Category="Data")
	bool PathfinderLog = false;
	
protected:
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UDataTable> UnitActorsTable_;
	
	UPROPERTY(Config, EditAnywhere, Category="Data")
	TSoftObjectPtr<UDataTable> UnitGhostActorsTable_;
};
