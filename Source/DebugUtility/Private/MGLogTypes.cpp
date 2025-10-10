#include "MGLogTypes.h"

#include "GameplayTagsManager.h"
#include "MGLogs.h"
#include "Utilities.h"

DEFINE_LOG_CATEGORY(ALogQuark)
DEFINE_LOG_CATEGORY_STATIC(ALoggingTypes, Log, All);

FGameplayTagContainer MGLogTypes::LogTags_;

void MGLogTypes::SetLogsFromString(const FString& InValue, const FString& InPrefix, FGameplayTagContainer& OutTags)
{
	if(InValue.IsEmpty())
		return;

	TArray<TTuple<int32, int32>> Array;
	ParseIntoArray(InValue, Array, TEXT('|'));

	TStringBuilder<512> sb;

	for (const auto& Tuple : Array)
	{
		const TCHAR* S = GetData(InValue) + Tuple.Key;
		int32 Len = Tuple.Value;
		
		sb.Reset();

		if(!InPrefix.IsEmpty())
		{
			sb.Append(InPrefix);
			TCHAR LastChar = InPrefix[InPrefix.Len() - 1]; 
			if(LastChar != TEXT('.'))
				sb.Append(TEXT("."));
		}
		
		sb.Append(S, Len);
		
		FName TagName(*sb);		

		FGameplayTag CurrentTag;
#if defined(ENGINE_MAJOR_VERSION) && (ENGINE_MAJOR_VERSION <= 4) && (ENGINE_MINOR_VERSION <= 26)		
		if(UGameplayTagsManager::Get().ImportSingleGameplayTag(CurrentTag, TagName))
#else			
		if(UGameplayTagsManager::Get().ImportSingleGameplayTag(CurrentTag, TagName, true))
#endif			
		{
			OutTags.AddTag(CurrentTag);
		}
		else
		{
			
			MG_ERROR(ALoggingTypes, TEXT("Bad Tag name: %s"), *sb);
		}
	}
}

template<typename EnumType>
void MGLogTypes::SetLogsFromStringEnum(const FString& InValue, uint32& OutValue)
{
	if(InValue.IsEmpty())
		return;

	static const FString StrAll(TEXT("All"));
	if(InValue.Equals(StrAll, ESearchCase::IgnoreCase))
	{
		OutValue = MGLogTypes::All;
		MG_LOG(ALoggingTypes, TEXT("Set Log Flag: All"));
		return;
	}
	
	UEnum* StartupEnum = StaticEnum<EnumType>();
	
	FStringView Str = FStringView(GetData(InValue), InValue.Len());
	FStringView EnumName;
	int32 Index = 0;
	TStringBuilder<128> sb;
	while(Str.FindChar(TEXT('|'), Index))
	{
		EnumName = Str.Left(Index);
		
		sb.Reset();
		sb.Append(EnumName.GetData(), EnumName.Len());

		int64 iVal = StartupEnum->GetValueByNameString(*sb);
		MG_COND_ERROR_SHORT(ALoggingTypes, iVal == INDEX_NONE);
		if(iVal != INDEX_NONE)
		{
			OutValue |= iVal;
			MG_LOG(ALoggingTypes, TEXT("Set Log Flag: %s"), *sb);
		}

		Index++;
		int32 Count = Str.Len() - Index;
		Str = FStringView(Str.GetData() + Index, Count);
		Index = 0;
	}

	EnumName = Str;
	sb.Reset();
	sb.Append(EnumName.GetData(), EnumName.Len());
	
	int64 iVal = StartupEnum->GetValueByNameString(*sb);
	MG_COND_ERROR_SHORT(ALoggingTypes, iVal == INDEX_NONE);
	if(iVal != INDEX_NONE)
	{
		OutValue |= iVal;
		MG_LOG(ALoggingTypes, TEXT("Set Log Flag: %s"), *sb);
	}
}

template void MGLogTypes::SetLogsFromStringEnum<EMGLogTypes>(const FString& InValue, uint32& OutValue);
template void MGLogTypes::SetLogsFromStringEnum<EMGChartTypes>(const FString& InValue, uint32& OutValue);