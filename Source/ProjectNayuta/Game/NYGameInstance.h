// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "NYGameInstance.generated.h"



UCLASS()
class PROJECTNAYUTA_API UNYGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
    UNYGameInstance();

    virtual void Init() override;

// Common
public:


protected:
    void OnNetworkFailure(UWorld* World, class UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString);

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Multiplay")
    FString LobbyMapPath = TEXT("/Game/Levels/LV_Lobby?listen");


// Session (Common & Internal)
public:
    UFUNCTION(BlueprintCallable, Category = "Network")
    void LeaveSession();

    UFUNCTION(BlueprintPure, Category = "Multiplay")
    FName GetCurrentSessionName() const { return CurrentSessionName; }

    UFUNCTION(BlueprintPure, Category = "Multiplay")
    int32 GetPendingMaxPlayers() const { return PendingMaxPlayers; }

protected:
    IOnlineSessionPtr SessionInterface;

    UPROPERTY(Transient, BlueprintReadOnly, Category = "Multiplay")
    FName CurrentSessionName;
    UPROPERTY(Transient, BlueprintReadOnly, Category = "Multiplay")
    int32 PendingMaxPlayers;

    // Common Delegate Handle
    // HostGame() Binding
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;
    FDelegateHandle LeaveSessionCompleteDelegateHandle;

    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
    void OnLeaveSessionComplete(FName SessionName, bool bWasSuccessful);


// Host & Join (General)
public:
    UFUNCTION(BlueprintCallable, Category = "Multiplay")
    void HostGame(FName SessionName, int32 MaxPlayerNum);

    UFUNCTION(BlueprintCallable, Category = "Multiplay")
    void FindAndJoinSession();

protected:
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    // FindAndJoinSession() Binding
    FDelegateHandle FindSessionsCompleteDelegateHandle; 

    // OnFindSessionsComplete() Binding
    // OnSessionUserInviteAccepted() Binding
    FDelegateHandle JoinSessionCompleteDelegateHandle;

    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


// Steam Specific (Invite & Presence)
public:

protected:
    // Steam Friend Invitation Accept Delegate
    // Init() Binding
    FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

    /**
     * @param bWasSuccessful Invitation Accept Success or Not
     * @param ControllerId Accepted Local Controller ID
     * @param UserId Invited User ID
     * @param InviteResult Invited Session Details
     */
    void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);


};