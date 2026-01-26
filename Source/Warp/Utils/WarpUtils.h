#pragma once

UENUM(BlueprintType)
enum class EClientEnv : uint8
{
	NotAClient,
	ClientPIE,
	ClientGame
};


UENUM(BlueprintType)
enum class ELaunchNetMode : uint8
{
	Standalone,
	Client,
	ListenServer,
	DedicatedServer
};

struct FLaunchContext
{	
	bool bIsEditor = false;
	bool bIsPIE = false;
	ELaunchNetMode NetMode = ELaunchNetMode::Standalone;
};

static FLaunchContext BuildLaunchContext(const UWorld* World)
{
	FLaunchContext Context;

#if WITH_EDITOR
	Context.bIsEditor = GIsEditor;
	if (World)
	{
		Context.bIsPIE = (World->WorldType == EWorldType::PIE);
	}
#endif

	if (!World)
	{
		return Context;
	}

	switch (World->GetNetMode())
	{
	case NM_Standalone:     Context.NetMode = ELaunchNetMode::Standalone;     break;
	case NM_Client:         Context.NetMode = ELaunchNetMode::Client;         break;
	case NM_ListenServer:   Context.NetMode = ELaunchNetMode::ListenServer;   break;
	case NM_DedicatedServer:Context.NetMode = ELaunchNetMode::DedicatedServer;break;
	default:                Context.NetMode = ELaunchNetMode::Standalone;     break;
	}

	return Context;
}

static bool IsAuthorityLike(const FLaunchContext& Ctx)
{
	switch (Ctx.NetMode)
	{
	case ELaunchNetMode::DedicatedServer:
	case ELaunchNetMode::ListenServer:
	case ELaunchNetMode::Standalone:
		return true;

	case ELaunchNetMode::Client:
	default:
		return false;
	}
}

static EClientEnv GetClientEnv(const FLaunchContext& Ctx)
{
	if (Ctx.NetMode != ELaunchNetMode::Client)
		return EClientEnv::NotAClient;

#if WITH_EDITOR
	if (Ctx.bIsPIE)
		return EClientEnv::ClientPIE;
#endif

	return EClientEnv::ClientGame;
}
