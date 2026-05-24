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

    // Network Failure Binding
    if (GEngine)
    {
        GEngine->OnNetworkFailure().AddUObject(this, &UNYGameInstance::OnNetworkFailure);
    }

}

void UNYGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type FailureType, const FString& ErrorString)
{
    // debug
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Network Error: %s"), *ErrorString));
    }

    if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(LeaveSessionCompleteDelegateHandle);

        LeaveSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnLeaveSessionComplete));
        SessionInterface->DestroySession(NAME_GameSession);
    }
    else
    {
        if (APlayerController* PC = GetFirstLocalPlayerController())
        {
            PC->ClientTravel("/Game/Maps/LV_MainMenu", TRAVEL_Absolute);
        }
    }

    //CurrentSessionName = FName();
    //PendingMaxPlayers = 0;

    //if (APlayerController* PC = GetFirstLocalPlayerController())
    //{
    //    PC->ClientTravel("/Game/Maps/LV_MainMenu", TRAVEL_Absolute);
    //}
}

void UNYGameInstance::LeaveSession()
{
    if (SessionInterface.IsValid() && SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(LeaveSessionCompleteDelegateHandle);

        LeaveSessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnLeaveSessionComplete));

        SessionInterface->DestroySession(NAME_GameSession);
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
    if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
    {
        SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);

        DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnDestroySessionComplete));

        SessionInterface->DestroySession(NAME_GameSession);

        return;
    }

    FOnlineSessionSettings SessionSettings;
    bool bIsLAN = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
    SessionSettings.bIsLANMatch = bIsLAN;
    SessionSettings.bAllowJoinViaPresence = !bIsLAN;
    SessionSettings.bUsesPresence = !bIsLAN;

    SessionSettings.NumPublicConnections = PendingMaxPlayers;
    SessionSettings.bAllowJoinInProgress = true;
    SessionSettings.bShouldAdvertise = true;
    SessionSettings.bUseLobbiesIfAvailable = true;

    SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);

    CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnCreateSessionComplete));

    SessionInterface->CreateSession(0, NAME_GameSession, SessionSettings);

}

// [Steam Specific] 초대 수락 콜백
void UNYGameInstance::OnSessionUserInviteAccepted(bool bWasSuccessful, int32 ControllerId, TSharedPtr<const FUniqueNetId> UserId, const FOnlineSessionSearchResult& InviteResult)
{
    if (!bWasSuccessful || !SessionInterface.IsValid())
        return;

    SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

    JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnJoinSessionComplete));

    // 초대를 통해 전달받은 세션 정보(InviteResult)로 바로 조인 시도
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
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

        JoinSessionCompleteDelegateHandle = SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnJoinSessionComplete));

        SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[0]);
    }
    else
    {
        if (GEngine)
        {
            FString FailReason = !bWasSuccessful ? TEXT("Search Failed") : TEXT("No Sessions Found");
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FailReason);
        }
    }
}

void UNYGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
    if (SessionInterface.IsValid())
        SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);

    if (Result == EOnJoinSessionCompleteResult::Success)
    {
        CurrentSessionName = SessionName;

        FString ConnectString;
        if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
        {
            if (APlayerController* PC = GetFirstLocalPlayerController())
                PC->ClientTravel(ConnectString, TRAVEL_Absolute);
        }
    }
    else
    {
        if (GEngine)
        {
            FString ErrorReason = TEXT("Unknown Error");
            if (Result == EOnJoinSessionCompleteResult::AlreadyInSession) ErrorReason = TEXT("Already In Session");
            else if (Result == EOnJoinSessionCompleteResult::SessionDoesNotExist) ErrorReason = TEXT("Session Does Not Exist");
            else if (Result == EOnJoinSessionCompleteResult::SessionIsFull) ErrorReason = TEXT("Session Is Full");

            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Join Failed: %s"), *ErrorReason));
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

    if (SessionSearch.IsValid() && SessionSearch->SearchState == EOnlineAsyncTaskState::InProgress)
    {
        if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Orange, TEXT("Search already in progress..."));
        return;
    }

    SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);

    FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNYGameInstance::OnFindSessionsComplete));

    SessionSearch = MakeShareable(new FOnlineSessionSearch());
    SessionSearch->MaxSearchResults = 10000;

    bool bIsLAN = (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL");
    SessionSearch->bIsLanQuery = bIsLAN;
    if (!bIsLAN)
    {
        SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

        SessionSearch->QuerySettings.Set(SEARCH_LOBBIES, true, EOnlineComparisonOp::Equals);
    }

    SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());


}