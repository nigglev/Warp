// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitDataSubsystem.h"
#include "HttpModule.h"
#include "JsonObjectConverter.h"
#include "MGLogs.h"
#include "Interfaces/IHttpResponse.h"
#include "Warp/Base/GameState/WarpGameState.h"
#include "Warp/UnitStaticData/PlayFabUnitTypes.h"
#include "Warp/UnitStaticData/UnitCatalogDTO.h"

DEFINE_LOG_CATEGORY_STATIC(AUnitDataSubsystemLog, Log, All);

static FString PFUrl(const FString& TitleId, const FString& Path)
{
	return FString::Printf(TEXT("https://%s.playfabapi.com/%s"), *TitleId, *Path);
}

void UUnitDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	StartAuthAndLoadUnits();
}

FUnitRecord UUnitDataSubsystem::GetBasicUnitRecord() const
{
    FUnitRecord UnitRecord = RemoteUnits["Corvette"];
    return UnitRecord;
}

void UUnitDataSubsystem::StartAuthAndLoadUnits()
{
	const ENetMode NM = GetWorld() ? GetWorld()->GetNetMode() : NM_Standalone;
    const bool bIsServer = (NM != NM_Client);

    if (!bIsServer)
    {
        MG_COND_LOG(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("UnitDataSubsystem: client; skipping PlayFab fetch (server authoritative)"));
    }
	
    // FString Secret = TEXT("HYBF1BJ3FA8PC3PWPNC395H9X9WWTBFX5WQU4SPKE9BMZANIR1");
    // if (Secret.IsEmpty())
    // {
    //     MG_COND_LOG(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
    //         TEXT("UnitDataSubsystem: PLAYFAB_SECRET_KEY env var is missing. Cannot fetch catalog."));
    //     return;
    // }
    
    FString Secret;
    if (!ReadPlayFabSecretFromFile(Secret))
    {
        MG_COND_LOG(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem), TEXT("UnitDataSubsystem: PlayFab secret missing. "
            "Expected: Saved/Secrets/PlayFab.ini (section [PlayFab], key SecretKey) "
            "or Saved/Secrets/PlayFabSecret.txt"));
        return;
    }
	
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(PFUrl(PlayFabTitleId, TEXT("Authentication/GetEntityToken")));
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetHeader(TEXT("X-SecretKey"), Secret);
    Req->SetContentAsString(TEXT("{}"));
    Req->OnProcessRequestComplete().BindLambda([this](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
    {
        if (!bOk || !Resp.IsValid())
        {
            MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("GetEntityToken: no response"));
            return;
        }
        if (!EHttpResponseCodes::IsOk(Resp->GetResponseCode()))
        {
            MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("GetEntityToken: HTTP %d - %s"), Resp->GetResponseCode(), *Resp->GetContentAsString());
            return;
        }

        FString EntityToken;
        {
            TSharedPtr<FJsonObject> J;
            FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Resp->GetContentAsString()), J);
            const TSharedPtr<FJsonObject>* Data = nullptr;
            if (J.IsValid() && J->TryGetObjectField(TEXT("data"), Data))
            {
                (*Data)->TryGetStringField(TEXT("EntityToken"), EntityToken);
            }
        }

        if (EntityToken.IsEmpty())
        {
            MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("GetEntityToken: missing token in response"));
            return;
        }

        FetchUnitsFromCatalog(EntityToken);
    });
    Req->ProcessRequest();
}

void UUnitDataSubsystem::FetchUnitsFromCatalog(const FString& InEntityToken, const FString& InContinuation)
{
    if (InContinuation.IsEmpty())
    {
        RemoteUnits.Empty();
    }

    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();
    Req->SetURL(PFUrl(PlayFabTitleId, TEXT("Catalog/SearchItems")));
    Req->SetVerb(TEXT("POST"));
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetHeader(TEXT("X-EntityToken"), InEntityToken);
    
    TSharedPtr<FJsonObject> BodyObj = MakeShared<FJsonObject>();
    BodyObj->SetStringField(TEXT("Filter"), TEXT("ContentType eq 'Unit'"));
    BodyObj->SetNumberField(TEXT("Count"), 50);
    if (!InContinuation.IsEmpty())
    {
        BodyObj->SetStringField(TEXT("ContinuationToken"), InContinuation);
    }

    FString Body;
    auto Writer = TJsonWriterFactory<>::Create(&Body);
    FJsonSerializer::Serialize(BodyObj.ToSharedRef(), Writer);
    Req->SetContentAsString(Body);

    Req->OnProcessRequestComplete().BindWeakLambda(this, ([this, InEntityToken](FHttpRequestPtr, FHttpResponsePtr Resp, bool bOk)
    {
        if (!bOk || !Resp.IsValid() || !EHttpResponseCodes::IsOk(Resp->GetResponseCode()))
        {
            MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("SearchItems failed: HTTP %d - %s"),
                   Resp.IsValid() ? Resp->GetResponseCode() : -1,
                   Resp.IsValid() ? *Resp->GetContentAsString() : TEXT("<no body>"));
            return;
        }
        
        ParseUnitsFromSearchResponse(Resp->GetContentAsString());
        
        FString ContinuationToken;
        {
            TSharedPtr<FJsonObject> Root;
            FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Resp->GetContentAsString()), Root);
            const FJsonObject* Payload = Root.Get();
            if (const TSharedPtr<FJsonObject>* DataObj = nullptr;
                Root.IsValid() && Root->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
            {
                Payload = DataObj->Get();
            }
            if (Payload)
            {
                if (!Payload->TryGetStringField(TEXT("ContinuationToken"), ContinuationToken))
                {
                    Payload->TryGetStringField(TEXT("continuationToken"), ContinuationToken);
                }
            }
        }
        
        if (!ContinuationToken.IsEmpty())
        {
            FetchUnitsFromCatalog(InEntityToken, ContinuationToken);
        }
        else
        {
            MG_COND_LOG(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("PlayFab Catalog loaded: %d Unit item(s) total."), RemoteUnits.Num());

            if (AWarpGameState* GS = GetWorld() ? GetWorld()->GetGameState<AWarpGameState>() : nullptr)
            {
                GS->SetUnitCatalogFromMap(RemoteUnits);
            }

            bUnitCatalogReady = true;
            OnUnitCatalogReady.Broadcast(true);
        }
    }));

    Req->ProcessRequest();
}

void UUnitDataSubsystem::ParseUnitsFromSearchResponse(const FString& InResponseJson)
{
     TSharedPtr<FJsonObject> Root;
    if (!FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(InResponseJson), Root) || !Root.IsValid())
    {
        MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("ParseUnits: invalid JSON"));
        return;
    }
    
    const FJsonObject* Payload = Root.Get();
    if (const TSharedPtr<FJsonObject>* DataObj = nullptr;
        Root->TryGetObjectField(TEXT("data"), DataObj) && DataObj && DataObj->IsValid())
    {
        Payload = DataObj->Get();
    }
    
    const TArray<TSharedPtr<FJsonValue>>* Items = nullptr;
    if (!Payload->TryGetArrayField(TEXT("Items"), Items))
    {
        MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("ParseUnits: no Items[] found"));
        return;
    }

    for (const TSharedPtr<FJsonValue>& V : *Items)
    {
        const TSharedPtr<FJsonObject> Item = V->AsObject();
        if (!Item) continue;
        
        FString Id = Item->GetStringField(TEXT("Id"));
        FString FriendlyId;
        if (const TArray<TSharedPtr<FJsonValue>>* AltIds = nullptr;
            Item->TryGetArrayField(TEXT("AlternateIds"), AltIds))
        {
            for (const auto& A : *AltIds)
            {
                const TSharedPtr<FJsonObject> AO = A->AsObject();
                if (!AO) continue;
                FString Type, Value;
                AO->TryGetStringField(TEXT("Type"), Type);
                AO->TryGetStringField(TEXT("Value"), Value);
                if (Type.Equals(TEXT("FriendlyId"), ESearchCase::IgnoreCase) && !Value.IsEmpty())
                {
                    FriendlyId = Value;
                    break;
                }
            }
        }

        const TSharedPtr<FJsonObject>* DP = nullptr;
        if (!Item->TryGetObjectField(TEXT("DisplayProperties"), DP) || !DP) continue;

        FUnitRecord Rec;
        Rec.UnitName = FName(*(!FriendlyId.IsEmpty() ? FriendlyId : Id));

        FPlayFabUnitDisplayProps Props;
        if (!FJsonObjectConverter::JsonObjectToUStruct(
                DP->ToSharedRef(),
                FPlayFabUnitDisplayProps::StaticStruct(),
                &Props,
                0, 0))
        {
            MG_COND_ERROR(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
            TEXT("Failed to convert DisplayProperties for %s"), *Rec.UnitName.ToString());
            return;
        }

        Rec.Props = MoveTemp(Props);
        RemoteUnits.Add(Rec.UnitName, MoveTemp(Rec));
    }

    MG_COND_LOG(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
           TEXT("Loaded %d unit(s) from PlayFab Catalog (page)."), RemoteUnits.Num());
}

void UUnitDataSubsystem::SetUnitCatalog_Client(const TArray<FUnitRecordDTO>& InUnitDTO)
{
    RemoteUnits.Empty(InUnitDTO.Num());
    for (const FUnitRecordDTO& DTO : InUnitDTO)
    {
        FUnitRecord Rec;
        Rec.UnitName = DTO.UnitName;
        Rec.Props.UnitSize = DTO.UnitSize;
        Rec.Props.UnitSpeed = DTO.UnitSpeed;
        Rec.Props.UnitMaxAP = DTO.UnitMaxAP;
        Rec.Props.MeshPath = FSoftObjectPath(DTO.MeshPath);
        Rec.Props.Tags = DTO.Tags;

        RemoteUnits.Add(Rec.UnitName, MoveTemp(Rec));
    }
    MG_COND_LOG(AUnitDataSubsystemLog, MGLogTypes::IsLogAccessed(EMGLogTypes::UnitDataSubsystem),
           TEXT("Unit catalog synced to subsystem on client. Count=%d"), RemoteUnits.Num());
}

bool UUnitDataSubsystem::ReadPlayFabSecretFromFile(FString& OutSecret) const
{
    const FString SecretIni = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("Secrets/PlayFab.ini"));
    
    if (GConfig->GetString(TEXT("PlayFab"), TEXT("SecretKey"), OutSecret, SecretIni))
    {
        OutSecret.TrimStartAndEndInline();
        OutSecret.ReplaceInline(TEXT("\""), TEXT(""));
        if (!OutSecret.IsEmpty())
        {
            return true;
        }
    }
    
    return false;
}