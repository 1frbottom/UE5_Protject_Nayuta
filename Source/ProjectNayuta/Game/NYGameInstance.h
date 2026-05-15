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

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Network")
    FString LobbyMapPath = TEXT("/Game/Levels/LV_Lobby?listen");


// Session (Common & Internal)
public:

protected:
    IOnlineSessionPtr SessionInterface;
    FName CurrentSessionName;
    int32 PendingMaxPlayers;

    // 공통 델리게이트 핸들
    // HostGame()에서 바인딩
    FDelegateHandle CreateSessionCompleteDelegateHandle;
    FDelegateHandle DestroySessionCompleteDelegateHandle;

    void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
    void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);


// Host & Join (General)
public:
    UFUNCTION(BlueprintCallable, Category = "Network")
    void HostGame(FName SessionName, int32 MaxPlayers);

    UFUNCTION(BlueprintCallable, Category = "Network")
    void FindAndJoinSession();

protected:
    TSharedPtr<class FOnlineSessionSearch> SessionSearch;

    // FindAndJoinSession()에서 바인딩
    FDelegateHandle FindSessionsCompleteDelegateHandle; 

    // OnFindSessionsComplete()에서 바인딩
    // OnSessionUserInviteAccepted()에서 바인딩
    FDelegateHandle JoinSessionCompleteDelegateHandle;

    void OnFindSessionsComplete(bool bWasSuccessful);
    void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);


// Steam Specific (Invite & Presence)
public:

protected:
    // 스팀 친구 초대 수락 시 실행될 델리게이트
    // Init()에서 바인딩
    FDelegateHandle SessionUserInviteAcceptedDelegateHandle;

    /**
     * @param bWasSuccessful 초대 수락 성공 여부
     * @param ControllerId 수락한 로컬 컨트롤러 ID
     * @param UserId 초대 보낸 유저의 ID
     * @param InviteResult 초대된 세션의 상세 정보
     */
    void OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult);


};