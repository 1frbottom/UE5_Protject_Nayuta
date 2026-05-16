// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYGameInstance.h"

#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Online/OnlineSessionNames.h"



UNYGameInstance::UNYGameInstance()
{

}

void UNYGameInstance::Init()
{
    Super::Init();

    if (IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get())
    {
        SessionInterface = Subsystem->GetSessionInterface();
        if (SessionInterface.IsValid())
        {
            // 스팀 초대 전용 델리게이트 바인딩
            // 상시 대기 상태여야하기 때문
            SessionUserInviteAcceptedDelegateHandle = SessionInterface->AddOnSessionUserInviteAcceptedDelegate_Handle(
                FOnSessionUserInviteAcceptedDelegate::CreateUObject(this, &UNYGameInstance::OnSessionUserInviteAccepted)
            );


        }
    }


}

void UNYGameInstance::LeaveSession()
{
    if (SessionInterface.IsValid() && !CurrentSessionName.IsNone())
    {
        LeaveSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnLeaveSessionComplete));

        SessionInterface->DestroySession(CurrentSessionName);
    }
    else
    {
        // 세션이 없거나 유효하지 않으면 바로 초기화 후 이동
        CurrentSessionName = FName();
        PendingMaxPlayers = 0;

        if (APlayerController* PC = GetFirstLocalPlayerController())
            PC->ClientTravel("/Game/Maps/LV_MainMenu", TRAVEL_Absolute);
    }
}

void UNYGameInstance::OnLeaveSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(LeaveSessionCompleteDelegateHandle);

    CurrentSessionName = FName();
    PendingMaxPlayers = 0;

    if (APlayerController* PC = GetFirstLocalPlayerController())
        PC->ClientTravel("/Game/Maps/LV_MainMenu", TRAVEL_Absolute);

}

void UNYGameInstance::HostGame(FName SessionName, int32 MaxPlayers)
{
    if (!SessionInterface.IsValid())
        return;

    CurrentSessionName = SessionName;
    PendingMaxPlayers = MaxPlayers;

    // 이미 방이 있다면
    if (SessionInterface->GetNamedSession(CurrentSessionName) != nullptr)
    {
        DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnDestroySessionComplete));

        SessionInterface->DestroySession(CurrentSessionName);

        return;
    }

    FOnlineSessionSettings SessionSettings;
    SessionSettings.bIsLANMatch = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");  // NULL 서브시스템(에디터)인 경우 LAN 매치로 설정
    SessionSettings.NumPublicConnections = PendingMaxPlayers;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bAllowJoinViaPresence = true; // 스팀 초대를 위해 필수
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUsesPresence = true;         // 스팀 상태 정보 활용

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnCreateSessionComplete));

    SessionInterface->CreateSession(0, CurrentSessionName, SessionSettings);

}

// [Steam Specific] 초대 수락 콜백
void UNYGameInstance::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
    if (!bWasSuccessful || !SessionInterface.IsValid())
        return;

    // 초대를 통해 전달받은 세션 정보(InviteResult)로 바로 조인 시도
    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnJoinSessionComplete));

    SessionInterface->JoinSession(ControllerId, NAME_GameSession, InviteResult);


}

void UNYGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

    // debug
    if (bWasSuccessful)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Session Success! Traveling..."));
    }
    else
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Session Creation Failed!"));
    }

    if (bWasSuccessful)
        GetWorld()->ServerTravel(LobbyMapPath);


}

void UNYGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

    if (bWasSuccessful && SessionSearch->SearchResults.Num() > 0)
    {
        JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnJoinSessionComplete));

        SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
    }


}

void UNYGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            if (APlayerController* PC = GetFirstLocalPlayerController())
                PC->ClientTravel(ConnectString, TRAVEL_Absolute);
        }
    }


}

void UNYGameInstance::OnDestroySessionComplete(FName SessionName, bool bWasSuccessful)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

    if (bWasSuccessful)
        HostGame(CurrentSessionName, PendingMaxPlayers);


}

void UNYGameInstance::FindAndJoinSession()
{
    if (!SessionInterface.IsValid())
        return;

    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnFindSessionsComplete));

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->bIsLanQuery = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
    SessionSearch->MaxSearchResults = 10000;
    SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());


}