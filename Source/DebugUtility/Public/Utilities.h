#pragma once
#include "CoreMinimal.h"

//String.cpp.inl int32 UE_STRING_CLASS::ParseIntoArray

inline void DEBUGUTILITY_API ParseIntoArray(const FString& InString, TArray<TTuple<int32, int32>>& OutArray, const TCHAR* InDelimArray, int32 InDelimLen)
{
	// Make sure the delimit string is not null or empty
	check(InDelimArray);
	check(InDelimLen > 0);
	OutArray.Reset();
	
	const TCHAR* Start = GetData(InString);
	const int32 Length = InString.Len();
	if(Start == nullptr || Length == 0)
		return;
	
	int32 SubstringBeginIndex = 0;

	// Iterate through string.
	for(int32 i = 0; i < Length;)
	{
		int32 SubstringEndIndex = INDEX_NONE;
		
		// If we found a delimiter...
		if (TCString<TCHAR>::Strncmp(Start + i, InDelimArray, InDelimLen) == 0)
		{
			// Mark the end of the substring.
			SubstringEndIndex = i;
		}

		if (SubstringEndIndex != INDEX_NONE)
		{
			const int32 SubstringLength = SubstringEndIndex - SubstringBeginIndex;
			// If we're not culling empty strings or if we are but the string isn't empty anyways...
			if(SubstringLength != 0)
			{
				TTuple<int32, int32>& Tp = OutArray.Emplace_GetRef();
				Tp.Key = SubstringBeginIndex;
				Tp.Value = SubstringEndIndex - SubstringBeginIndex;
			}
			// Next substring begins at the end of the discovered delimiter.
			SubstringBeginIndex = SubstringEndIndex + InDelimLen;
			i = SubstringBeginIndex;
		}
		else
		{
			++i;
		}
	}

	// Add any remaining characters after the last delimiter.
	const int32 SubstringLength = Length - SubstringBeginIndex;
	// If we're not culling empty strings or if we are but the string isn't empty anyways...
	if(SubstringLength != 0)
	{
		TTuple<int32, int32>& Tp = OutArray.Emplace_GetRef();
		Tp.Key = SubstringBeginIndex;
		Tp.Value = SubstringLength;
	}
}

inline void DEBUGUTILITY_API ParseIntoArray(const FString& InString, TArray<TTuple<int32, int32>>& OutArray, TCHAR InDelimeter)
{
	ParseIntoArray(InString, OutArray, &InDelimeter, 1);
}