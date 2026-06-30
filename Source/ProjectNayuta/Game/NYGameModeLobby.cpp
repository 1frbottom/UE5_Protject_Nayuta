// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameModeLobby.h"

#include "Game/NYGameInstance.h"
#include "Game/NYGameStateMainmenu.h"

ANYGameModeLobby::ANYGameModeLobby()
{
	// Base sets true; keep explicit — Lobby -> Stage should seamless-travel connected players.
	bUseSeamlessTravel = true;
}

void ANYGameModeLobby::InitGameState()
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
