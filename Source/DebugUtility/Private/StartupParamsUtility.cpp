#include "StartupParamsUtility.h"
#include "MGLogs.h"
#include "Misc/CommandLine.h"

DEFINE_LOG_CATEGORY_STATIC(LogStartupParamsUtility, Log, All);

FStartupParamsUtility::FStartupParamsUtility(const std::initializer_list<FName>& InParamNames)
	: FStartupParamsUtility(nullptr, InParamNames)
{}

FStartupParamsUtility::FStartupParamsUtility(const TCHAR* InEnvVarsValue, const std::initializer_list<FName>& InParamNames)
	: ParamNames_(InParamNames), EnvVarsName_(InEnvVarsValue)
{
	if(!EnvVarsName_.IsEmpty())
	{
		EnvVarsValue_ = FPlatformMisc::GetEnvironmentVariable(*EnvVarsName_);
		CollectValues(*EnvVarsValue_);
	}

	CommandLine_ = FCommandLine::Get();
	CollectValues(*CommandLine_);
}

void FStartupParamsUtility::CollectValues(const TCHAR* InString)
{
	RETURN_ON_FAIL(LogStartupParamsUtility, InString != nullptr);
	
	//UEnum* StartupEnum = StaticEnum<EStartupParams>(); 
	int32 EnumsNum = ParamNames_.Num();
	for(int32 Index = 0; Index < EnumsNum; ++Index)
	{
		FString EnumName = ParamNames_[Index].ToString();

		bool bAppended = false;
		{
			TStringBuilder<128> sbPV;
			sbPV.Append(EnumName).Append(TEXT("="));

			FString sVal;
			if(FParse::Value(InString, *sbPV, sVal))
			{
				if(sVal.IsNumeric())
				{
					if(sVal.GetCharArray().Contains(TEXT('.')))
					{
						double& fVal = FloatMap_.FindOrAdd(Index);
						fVal = FCString::Atod(*sVal);
						MG_LOG(LogStartupParamsUtility, TEXT("Float Param: %s=%f"), *EnumName, fVal);
					}
					else
					{
						int64& iVal = IntMap_.FindOrAdd(Index);
						iVal = FCString::Atoi64(*sVal);
						MG_LOG(LogStartupParamsUtility, TEXT("Int Param: %s=%d"), *EnumName, iVal);
					}
				}
				else
				{
					FString& sOldVal = StrMap_.FindOrAdd(Index);
					sOldVal = sVal;
					MG_LOG(LogStartupParamsUtility, TEXT("Str Param: %s=%s"), *EnumName, *sVal);
				}
				bAppended = true;
			}
		}

		if(!bAppended && !Keys_.Contains(Index))
		{
			TStringBuilder<64> sbK;
			sbK.Append(TEXT("-")).Append(EnumName);

			bAppended = FCString::Stristr(InString, *sbK) != nullptr;
			if(bAppended)
			{
				Keys_.Add(Index);
				MG_LOG(LogStartupParamsUtility, TEXT("Add Key: -%s"), *EnumName);
			}
		}
	}
}

FString FStartupParamsUtility::GetErrorText(const FName& InStartupParams) const
{
	return FString::Printf(TEXT("Unknown Key: %s; CommandLine: %s; EnvVarsName: %s; EnvVarsValue: %s"),
		*InStartupParams.ToString(), *CommandLine_, *EnvVarsName_, *EnvVarsValue_);
}

double FStartupParamsUtility::GetValueAsFloat(const FName& InStartupParams, double InDefaultValue) const
{
	int32 Key = ParamNames_.IndexOfByKey(InStartupParams);
	MG_COND_ERROR(LogStartupParamsUtility, Key == INDEX_NONE, TEXT("%s"), *GetErrorText(InStartupParams));
	if(Key == INDEX_NONE)
		return InDefaultValue;
	
	if(FloatMap_.Contains(Key))
		return FloatMap_[Key];
	if(IntMap_.Contains(Key))
		return IntMap_[Key];
	return InDefaultValue;
}

int64 FStartupParamsUtility::GetValueAsInt(const FName& InStartupParams, int64 InDefaultValue) const
{
	int32 Key = ParamNames_.IndexOfByKey(InStartupParams);
	MG_COND_ERROR(LogStartupParamsUtility, Key == INDEX_NONE, TEXT("%s"), *GetErrorText(InStartupParams));
	if(Key == INDEX_NONE)
		return InDefaultValue;
	
	if(IntMap_.Contains(Key))
		return IntMap_[Key];
	return InDefaultValue;
}

const FString& FStartupParamsUtility::GetValueAsString(const FName& InStartupParams) const
{
	static FString EmptyStr;
	int32 Key = ParamNames_.IndexOfByKey(InStartupParams);
	MG_COND_ERROR(LogStartupParamsUtility, Key == INDEX_NONE, TEXT("%s"), *GetErrorText(InStartupParams));
	if(Key == INDEX_NONE)
		return EmptyStr;
	
	if(StrMap_.Contains(Key))
		return StrMap_[Key];
	
	return EmptyStr;
}

const FString& FStartupParamsUtility::GetValueAsString(const FName& InStartupParams, const FString& InDefault) const
{
	int32 Key = ParamNames_.IndexOfByKey(InStartupParams);
	MG_COND_ERROR(LogStartupParamsUtility, Key == INDEX_NONE, TEXT("%s"), *GetErrorText(InStartupParams));
	if(Key == INDEX_NONE)
		return InDefault;
	
	if(StrMap_.Contains(Key))
		return StrMap_[Key];
	
	return InDefault;
}

EStartupParamsType FStartupParamsUtility::GetStartupParamType(const FName& InStartupParams) const
{
	int32 Key = ParamNames_.IndexOfByKey(InStartupParams);
	MG_COND_ERROR(LogStartupParamsUtility, Key == INDEX_NONE, TEXT("%s"), *GetErrorText(InStartupParams));
	
	if(Key == INDEX_NONE)
		return EStartupParamsType::Unknown;
	
	if(IntMap_.Contains(Key)) return EStartupParamsType::Int32;
	if(FloatMap_.Contains(Key)) return EStartupParamsType::Float;
	if(StrMap_.Contains(Key)) return EStartupParamsType::String;
	return EStartupParamsType::Unknown;
}

bool FStartupParamsUtility::IsKeyPresent(const FName& InStartupParams) const
{
	int32 Key = ParamNames_.IndexOfByKey(InStartupParams);
	RETURN_ON_FAIL_BOOL_T(LogStartupParamsUtility, Key != INDEX_NONE, TEXT("%s"), *GetErrorText(InStartupParams));
	
	return Keys_.Contains(Key);
}