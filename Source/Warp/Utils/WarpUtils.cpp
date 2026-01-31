#include "WarpUtils.h"

FString FLaunchContext::ToString() const
{
	if (bIsPIE)
		return FString::Printf(TEXT("PIE: %s"), *StaticEnum<ELaunchNetMode>()->GetNameStringByValue(static_cast<int64>(NetMode)));
	if (bIsEditor)
		return TEXT("Editor");
	return FString::Printf(TEXT("%s"), *StaticEnum<ELaunchNetMode>()->GetNameStringByValue(static_cast<int64>(NetMode)));
}
