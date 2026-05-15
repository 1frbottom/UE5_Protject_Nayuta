// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameStateMainmenu.h"

#include "GameFramework/PlayerState.h"



TArray<APlayerState*> ANYGameStateMainmenu::GetSortedPlayerArray()
{
    TArray<APlayerState*> SortedArray = PlayerArray;

    // ascending sort by PlayerId
    SortedArray.Sort(
        [](const APlayerState& A, const APlayerState& B)
        {
        return A.GetPlayerId() < B.GetPlayerId();
        }
    );

    return SortedArray;
}

void ANYGameStateMainmenu::AddPlayerState(APlayerState* PlayerState)
{
    Super::AddPlayerState(PlayerState);

    OnPlayerListChangedDelegate.Broadcast();
}

void ANYGameStateMainmenu::RemovePlayerState(APlayerState* PlayerState)
{
    Super::RemovePlayerState(PlayerState);

    OnPlayerListChangedDelegate.Broadcast();
}
