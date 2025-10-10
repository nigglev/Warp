#pragma once
#include "CoreMinimal.h"

enum class EStartupParamsType : uint8
{
	Unknown = 0,
	Int32,
	Float,
	String
};

class DEBUGUTILITY_API FStartupParamsUtility
{
public:
	FStartupParamsUtility() = default;
	FStartupParamsUtility(const std::initializer_list<FName>& InParamNames);
	FStartupParamsUtility(const TCHAR* InEnvVarsValue, const std::initializer_list<FName>& InParamNames);	

	EStartupParamsType GetStartupParamType(const FName& InStartupParams) const;
	
	int64 GetValueAsInt(const FName& InStartupParams, int64 InDefaultValue) const;
	double GetValueAsFloat(const FName& InStartupParams, double InDefaultValue) const;
	const FString& GetValueAsString(const FName& InStartupParams) const;
	const FString& GetValueAsString(const FName& InStartupParams, const FString& InDefault) const;
	
	bool IsKeyPresent(const FName& InStartupParams) const;
	
private:

	void CollectValues(const TCHAR* InString);

	FString GetErrorText(const FName& InStartupParams) const;

	TArray<FName> ParamNames_;
	
	FString EnvVarsName_;
	FString EnvVarsValue_;
	FString CommandLine_;

	TMap<int32, int64> IntMap_;
	TMap<int32, FString> StrMap_;
	TMap<int32, double> FloatMap_;

	TSet<int32> Keys_;
};
