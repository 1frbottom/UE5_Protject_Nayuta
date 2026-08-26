// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/NYPlayerControllerInGame.h"

#include "Player/NYPlayerStateStage.h"



void ANYPlayerControllerInGame::RefreshPlayerHpUI()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	const ANYPlayerStateStage* PS = GetPlayerState<ANYPlayerStateStage>();
	if (!PS)
	{
		return;
	}

	const float MaxHp = PS->GetMaxHp();
	const float HpPercent = (MaxHp > 0.0f) ? (PS->GetCurrHp() / MaxHp) : 0.0f;
	UpdatePlayerHpUI(HpPercent);
}
