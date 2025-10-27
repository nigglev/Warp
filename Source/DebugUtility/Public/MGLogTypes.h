#pragma once
#include "CoreMinimal.h"
#include "GameplayTagContainer.h"

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMGLogTypes : uint32
{
	None					= 0,
	GameMode				= 1,
	GameState				= 2,
	PlayerController		= 3,
	CombatMap				= 4,
	UnitBase				= 5,
	CombatMapManager		= 6,
	DefaultPlayerController	= 7,
	DefaultWarpHUD			= 8,
	UnitSpawnWidget			= 9,
	TurnBasedSystemManager	= 10,
	UnitDataSubsystem		= 11,
};
ENUM_CLASS_FLAGS(EMGLogTypes)

UENUM(meta=(Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class EMGChartTypes : uint32
{
	None					= 0,
};
ENUM_CLASS_FLAGS(EMGChartTypes)

class DEBUGUTILITY_API MGLogTypes
{
public:
	inline static uint32 CurrentLogs = static_cast<uint64>(EMGLogTypes::None);
	inline static uint32 CurrentCharts = static_cast<uint64>(EMGChartTypes::None);
	
	static constexpr uint32 All = UINT32_MAX;

	inline static bool IsLogAccessed(EMGLogTypes InType)
	{
		uint32 Val = static_cast<uint32>(InType);
		return (Val & CurrentLogs) > 0;
	}

	inline static bool IsChartAccessed(EMGChartTypes InType)
	{
		uint32 Val = static_cast<uint32>(InType);
		return (Val & CurrentCharts) > 0;
	}

	template<typename EnumType>
	static void SetLogsFromStringEnum(const FString& InValue, uint32& OutValue);

	static void SetLogsFromString(const FString& InValue, const FString& InPrefix, FGameplayTagContainer& OutTags);

	static FGameplayTagContainer LogTags_;

	inline static bool IsLogAccessed(const FGameplayTag& InTag)
	{
		return LogTags_.HasTag(InTag);
	}
};
