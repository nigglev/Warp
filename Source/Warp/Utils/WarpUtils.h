#pragma once


static bool IsUEEditorActive()
{
#if WITH_EDITOR
	return GIsEditor && !IsRunningCommandlet();
#else
	return false;
#endif
}
