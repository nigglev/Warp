#pragma once

UENUM()
enum class EClientEnv : uint8
{
	NotAClient,
	ClientPIE,
	ClientGame
};


UENUM()
enum class ELaunchNetMode : uint8
{
	Standalone = 0,
	DedicatedServer,
	ListenServer,
	Client,
	MAX
};

struct FLaunchContext
{	
	bool bIsEditor = false;
	bool bIsPIE = false;
	ELaunchNetMode NetMode = ELaunchNetMode::Standalone;

	FString ToString() const;
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

	Context.NetMode = static_cast<ELaunchNetMode>(World->GetNetMode()); 

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
