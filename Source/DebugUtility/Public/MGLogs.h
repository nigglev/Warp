#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(ALogQuark, Log, All);

#ifndef __func__
#define __func__ __FUNCTION__
#endif

#ifndef MG_LOG_DENY
#define MG_LOG_DENY (UE_BUILD_TEST || UE_BUILD_SHIPPING)
#endif

#ifndef MG_LOG_DENY_0
#define MG_LOG_DENY_0 0
#endif

#ifndef CHECK_BREAK
#define CHECK_BREAK 0
#endif

constexpr bool IsCheckBreak() 
{ 
#if CHECK_BREAK
	return true; 
#else
	return false;
#endif
}

#if !defined(ENGINE_MAJOR_VERSION) || ENGINE_MAJOR_VERSION >= 5
#define MG_LOG_TIME_PRINT 0
#else
#define MG_LOG_TIME_PRINT 1
#endif

FString GetClassMethodName(const ANSICHAR* InName);

inline FString GetSDS(const TCHAR* InMethodName, int32 InLine, const TCHAR* InText)
{
#if MG_LOG_TIME_PRINT
	FDateTime CurrentDateTime = FDateTime::Now();
	return FString::Printf(TEXT("%02d:%02d:%03d: %s %d: %s"), CurrentDateTime.GetMinute(), CurrentDateTime.GetSecond(), CurrentDateTime.GetMillisecond(),
		InMethodName, InLine, InText);
#else
	return FString::Printf(TEXT("%s %d: %s"), InMethodName, InLine, InText);
#endif
}

inline FString GetSDSS(const TCHAR* InMethodName, int32 InLine, const TCHAR* InText1, const TCHAR* InText2)
{
#if MG_LOG_TIME_PRINT
	FDateTime CurrentDateTime = FDateTime::Now();
	return FString::Printf(TEXT("%02d:%02d:%03d: %s %d: %s (%s)"), CurrentDateTime.GetMinute(), CurrentDateTime.GetSecond(), CurrentDateTime.GetMillisecond(),
		InMethodName, InLine, InText1, InText2);
#else
	return FString::Printf(TEXT("%s %d: %s (%s)"), InMethodName, InLine, InText1, InText2);
#endif
}

inline FString GetSS(const TCHAR* InMethodName, const TCHAR* InText)
{
#if MG_LOG_TIME_PRINT
	FDateTime CurrentDateTime = FDateTime::Now();
	return FString::Printf(TEXT("%02d:%02d:%03d: %s: %s"), CurrentDateTime.GetMinute(), CurrentDateTime.GetSecond(), CurrentDateTime.GetMillisecond(),
		InMethodName, InText);
#else
	return FString::Printf(TEXT("%s: %s"), InMethodName, InText);
#endif
}

inline FString GetMethodName(const TCHAR* InMethodName)
{
#if MG_LOG_TIME_PRINT
	FDateTime CurrentDateTime = FDateTime::Now();
	return FString::Printf(TEXT("%02d:%02d:%03d: %s"), CurrentDateTime.GetMinute(), CurrentDateTime.GetSecond(), CurrentDateTime.GetMillisecond(),
		InMethodName);
#else
	return FString::Printf(TEXT("%s"), InMethodName);
#endif	
}

#define MG_ERROR(CategoryName, Format, ...) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	FString Text = FString::Printf(Format, ##__VA_ARGS__); \
	UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, *Text)); \
	if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
}

#define MG_COND_ERROR(CategoryName, Cond, Format, ...) \
{ \
	if (UNLIKELY(Cond)) { \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDSS(*MethodName, __LINE__, TEXT(#Cond), *Text)); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
	}\
}

#define MG_COND_ERROR_SHORT(CategoryName, Cond) \
{ \
	if (UNLIKELY(Cond)) { \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#Cond))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
	}\
}

#if MG_LOG_DENY
#define MG_FUNC_LABEL(CategoryName) {}
#else
#define MG_FUNC_LABEL(CategoryName) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	UE_LOG(CategoryName, Log, TEXT("%s"), *GetMethodName(*MethodName)); \
}
#endif

#if MG_LOG_DENY
#define MG_COND_FUNC_LABEL(CategoryName) {}
#else
#define MG_COND_FUNC_LABEL(CategoryName, Cond) \
{ \
	if (UNLIKELY(Cond)) { \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Log, TEXT("%s"), *GetMethodName(*MethodName)); \
	}\
}
#endif

#if MG_LOG_DENY
#define MG_TEXT(CategoryName, LogText) {}
#else
#define MG_TEXT(CategoryName, LogText) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	UE_LOG(CategoryName, Log, TEXT("%s"), *GetSS(*MethodName, LogText)); \
}
#endif

#if MG_LOG_DENY
#define MG_STRING(CategoryName, Format, ...) {}
#else
#define MG_STRING(CategoryName, LogText) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	UE_LOG(CategoryName, Log, TEXT("%s"), *GetSS(*MethodName, *LogText)); \
}
#endif

#if MG_LOG_DENY
#define MG_LOG(CategoryName, Format, ...) {}
#else
#define MG_LOG(CategoryName, Format, ...) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	FString Text = FString::Printf(Format, ##__VA_ARGS__); \
	UE_LOG(CategoryName, Log, TEXT("%s"), *GetSS(*MethodName, *Text)); \
}
#endif

#if MG_LOG_DENY
#define MG_COND_LOG(CategoryName, Cond, Format, ...) {}
#else
#define MG_COND_LOG(CategoryName, Cond, Format, ...) \
{ \
	if (UNLIKELY(Cond)) { \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Log, TEXT("%s"), *GetSS(*MethodName, *Text)); \
	}\
}
#endif

#if MG_LOG_DENY
#define MG_WARNING(CategoryName, Format, ...) {}
#else
#define MG_WARNING(CategoryName, Format, ...) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	FString Text = FString::Printf(Format, ##__VA_ARGS__); \
	UE_LOG(CategoryName, Warning, TEXT("%s"), *GetSDS(*MethodName, __LINE__, *Text)); \
}
#endif

#if MG_LOG_DENY
#define MG_WARNING_TEXT(CategoryName, TXT) {}
#else
#define MG_WARNING_TEXT(CategoryName, TXT) \
{ \
	FString MethodName = GetClassMethodName(__func__ ); \
	UE_LOG(CategoryName, Warning, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TXT)); \
}
#endif

#if MG_LOG_DENY
#define MG_COND_WARNING(CategoryName, Format, ...) {}
#else
#define MG_COND_WARNING(CategoryName, Cond, Format, ...) \
{ \
	if (UNLIKELY(Cond)) { \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Warning, TEXT("%s"), *GetSDS(*MethodName, __LINE__, *Text)); \
	}\
}
#endif

#if MG_LOG_DENY
#define MG_COND_WARNING_SHORT(CategoryName, Cond) {}
#else
#define MG_COND_WARNING_SHORT(CategoryName, Cond) \
{ \
	if (UNLIKELY(Cond)) { \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Warning, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#Cond))); \
	}\
}
#endif

// /// CHECK_RET
#if MG_LOG_DENY
#define RETURN_ON_FAIL(CategoryName, EXP) { if (!(EXP)) { return; } }
#else
#define RETURN_ON_FAIL(CategoryName, EXP) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#EXP))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_DEFAULT(CategoryName, EXP, DEF_VALUE) { if (!(EXP)) { return DEF_VALUE; } }
#else
#define RETURN_ON_FAIL_DEFAULT(CategoryName, EXP, DEF_VALUE) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#EXP))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return DEF_VALUE; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_NULL(CategoryName, EXP) { if (!(EXP)) { return nullptr; } }
#else
#define RETURN_ON_FAIL_NULL(CategoryName, EXP) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#EXP))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return nullptr; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_BOOL(CategoryName, EXP) { if (!(EXP)) { return false; } }
#else
#define RETURN_ON_FAIL_BOOL(CategoryName, EXP) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#EXP))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return false; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_0(CategoryName, EXP) { if (!(EXP)) { return 0; } }
#else
#define RETURN_ON_FAIL_0(CategoryName, EXP) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#EXP))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return 0; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_STR(CategoryName, EXP) { if (!(EXP)) { static FString empty_str; return empty_str; } }
#else
#define RETURN_ON_FAIL_STR(CategoryName, EXP) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDS(*MethodName, __LINE__, TEXT(#EXP))); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		static FString empty_str; \
		return empty_str; \
	} \
}
#endif

// /// CHECK_M_RET
#if MG_LOG_DENY
#define RETURN_ON_FAIL_T(CategoryName, EXP, Format, ...) { if (!(EXP)) { return; } }
#else
#define RETURN_ON_FAIL_T(CategoryName, EXP, Format, ...) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDSS(*MethodName, __LINE__, TEXT(#EXP), *Text)); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_NULL_T(CategoryName, EXP, Format, ...) { if (!(EXP)) { return nullptr; } }
#else
#define RETURN_ON_FAIL_NULL_T(CategoryName, EXP, Format, ...) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDSS(*MethodName, __LINE__, TEXT(#EXP), *Text)); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return nullptr; \
	} \
}
#endif

#if MG_LOG_DENY
#define RETURN_ON_FAIL_BOOL_T(CategoryName, EXP, Format, ...) { if (!(EXP)) { return false; } }
#else
#define RETURN_ON_FAIL_BOOL_T(CategoryName, EXP, Format, ...) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDSS(*MethodName, __LINE__, TEXT(#EXP), *Text)); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return false; \
	} \
}
#endif


#if MG_LOG_DENY
#define RETURN_ON_FAIL_0_T(CategoryName, EXP, Format, ...) { if (!(EXP)) { return 0; } }
#else
#define RETURN_ON_FAIL_0_T(CategoryName, EXP, Format, ...) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDSS(*MethodName, __LINE__, TEXT(#EXP), *Text)); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return 0; \
	} \
}
#endif


#if MG_LOG_DENY
#define RETURN_ON_FAIL_DEFAULT_T(CategoryName, EXP, DEF_VALUE, Format, ...) { if (!(EXP)) { return DEF_VALUE; } }
#else
#define RETURN_ON_FAIL_DEFAULT_T(CategoryName, EXP, DEF_VALUE, Format, ...) { \
	if (UNLIKELY(!(EXP))) \
	{ \
		FString MethodName = GetClassMethodName(__func__ ); \
		FString Text = FString::Printf(Format, ##__VA_ARGS__); \
		UE_LOG(CategoryName, Error, TEXT("%s"), *GetSDSS(*MethodName, __LINE__, TEXT(#EXP), *Text)); \
		if(IsCheckBreak()) { UE_DEBUG_BREAK(); } \
		return DEF_VALUE; \
	} \
}
#endif

inline FString GetClassMethodName(const ANSICHAR* InName)
{
	const ANSICHAR* p = TCString<ANSICHAR>::Strchr(InName, ':');
	if (p == nullptr || *(p + 1) != ':')
		p = InName;
	else
		p = p + 2;

	return FString(ANSI_TO_TCHAR(p));
}

#undef MG_LOG_TIME_PRINT