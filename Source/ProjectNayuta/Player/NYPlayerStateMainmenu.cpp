// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerStateMainmenu.h"

#include "Net/UnrealNetwork.h"

#include "Game/NYGameStateMainmenu.h"

#include "Player/NYPlayerControllerMainmenu.h"



void ANYPlayerStateMainmenu::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANYPlayerStateMainmenu, bIsReadyLobby);


}

void ANYPlayerStateMainmenu::SetIsReadyLobby(bool NewReady)
{
    if (HasAuthority())
    {
        bIsReadyLobby = NewReady;

        OnRep_bIsReadyLobby();
    }
}

void ANYPlayerStateMainmenu::OnRep_PlayerId()
{
    Super::OnRep_PlayerId();

    // 클라이언트에 PlayerId가 동기화된 직후 UI 새로고침 신호 전송
    if (ANYGameStateMainmenu* GS = Cast<ANYGameStateMainmenu>(GetWorld()->GetGameState()))
    {
        GS->OnPlayerListChangedDelegate.Broadcast();
    }
}

void ANYPlayerStateMainmenu::OnRep_bIsReadyLobby()
{
    OnReadyStateChangedDelegate.Broadcast(bIsReadyLobby);
}