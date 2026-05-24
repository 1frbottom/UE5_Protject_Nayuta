// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameModeMainmenu.h"

#include "Game/NYGameInstance.h"
#include "Game/NYGameStateMainmenu.h"



void ANYGameModeMainmenu::InitGameState()
{
    Super::InitGameState();

    UNYGameInstance* GI = Cast<UNYGameInstance>(GetGameInstance());
    ANYGameStateMainmenu* GS = Cast<ANYGameStateMainmenu>(GameState);
    if (GI && GS)
    {
        GS->CurrentSessionName = GI->GetCurrentSessionName();
        GS->MaxPlayers = GI->GetPendingMaxPlayers();
    }
}
