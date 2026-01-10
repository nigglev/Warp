// Fill out your copyright notice in the Description page of Project Settings.


#include "WarpPlayfabContentSubSystem.h"
#include "MGLogs.h"
#include "Core/PlayFabClientAPI.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Warp/ContentManagement/ContentManagementStates/States/PlayFabStateManager.h"


DEFINE_LOG_CATEGORY_STATIC(ContentLog, Log, All);

UWarpPlayfabContentSubSystem::UWarpPlayfabContentSubSystem()
{
    StateManager_ = CreateDefaultSubobject<UPlayFabStateManager>(TEXT("PFStateManager"));
}

UWarpPlayfabContentSubSystem* UWarpPlayfabContentSubSystem::Get(const UObject* WorldContextObject)
{
    UWorld* World = WorldContextObject->GetWorld();
    RETURN_ON_FAIL_NULL(ContentLog, World);
    
    UGameInstance* GI = World->GetGameInstance();
    RETURN_ON_FAIL_NULL(ContentLog, GI);
    
    auto Content =	GI->GetSubsystem<UWarpPlayfabContentSubSystem>();
    RETURN_ON_FAIL_NULL(ContentLog, Content);

    return Content;
}

void UWarpPlayfabContentSubSystem::Initialize(FSubsystemCollectionBase& InCollection)
{
    Super::Initialize(InCollection);
    StateManager_->SetOwner(this);
    StateManager_->SetState(EPlayFabContentStates::StartLogin);
}

void UWarpPlayfabContentSubSystem::LoginToPlayFab()
{
    if (StateManager_->GetState() == EPlayFabContentStates::None || StateManager_->GetState() == EPlayFabContentStates::LoginFailure)
        StateManager_->SetState(EPlayFabContentStates::StartLogin);
    else
    {
        MG_WARNING(ContentLog, TEXT("Already logged in"));
    }
}

void UWarpPlayfabContentSubSystem::SaveDescriptionToPlayFab(const FString& InDescriptionName)
{
    if (IsClient())
    {
        MG_ERROR(ContentLog, TEXT("Can't save to PlayFab from client"));
        return;
    }
   
    if (StateManager_->GetState() == EPlayFabContentStates::UpdatePending)
    {
        MG_ERROR(ContentLog, TEXT("Please update to latest version before saving to PlayFab"));
        return;
    }
    FPlayFabStateManagerData StateData;
    StateData.DescriptionName = InDescriptionName;
    StateManager_->SetState(EPlayFabContentStates::SaveDescriptions, &StateData);
    
}

void UWarpPlayfabContentSubSystem::UpdateContent()
{
    if (!IsClient())
    {
        if (StateManager_->GetState() != EPlayFabContentStates::UpdatePending)
        {
            MG_WARNING(ContentLog, TEXT("Nothing to update; You have latest version"));
        }
        StateManager_->SetState(EPlayFabContentStates::GettingOutdatedContent);    
    }
}


void UWarpPlayfabContentSubSystem::OnPlayFabError(const PlayFab::FPlayFabCppError& ErrorResult)
{
    UE_LOG(LogTemp, Error, TEXT("PlayFab error: %s"),
           *ErrorResult.GenerateErrorReport());
}

bool UWarpPlayfabContentSubSystem::IsClient() const
{
    ENetMode NetMode = GetWorld()->GetNetMode();
    return  (NetMode == NM_Client);
}

void UWarpPlayfabContentSubSystem::BroadcastContentIsLoaded(bool InbIsContentLoaded)
{
    bUnitsLoaded_ = InbIsContentLoaded;
    OnUnitsLoaded.Broadcast();
}
