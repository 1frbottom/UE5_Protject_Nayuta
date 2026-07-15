// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYGameModeTraining.h"

#include "UObject/ConstructorHelpers.h"

#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Game/NYGameStateStage.h"
#include "Player/NYPlayerControllerStage.h"
#include "Player/NYPlayerStateStage.h"
#include "Weapons/NYWeaponComponent.h"
#include "Weapons/NYWeaponDefinition.h"

ANYGameModeTraining::ANYGameModeTraining()
{
	// Local sandbox — no lobby/session travel.
	bUseSeamlessTravel = false;

	static ConstructorHelpers::FClassFinder<APawn> PawnBPClass(
		TEXT("/Game/Characters/Player/BP_CharacterPlayer"));
	if (PawnBPClass.Succeeded())
	{
		DefaultPawnClass = PawnBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerController> PCBPClass(
		TEXT("/Game/Player/BP_PlayerController_Stage"));
	if (PCBPClass.Succeeded())
	{
		PlayerControllerClass = PCBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APlayerState> PSBPClass(
		TEXT("/Game/Player/BP_PlayerState_Stage"));
	if (PSBPClass.Succeeded())
	{
		PlayerStateClass = PSBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<AGameStateBase> GSBPClass(
		TEXT("/Game/Game/BP_GameState_Stage"));
	if (GSBPClass.Succeeded())
	{
		GameStateClass = GSBPClass.Class;
	}

	static ConstructorHelpers::FObjectFinder<UNYWeaponDefinition> DefaultWeaponAsset(
		TEXT("/Game/Data/DA_Weapon_Axe"));
	if (DefaultWeaponAsset.Succeeded())
	{
		DefaultWeaponDefinition = DefaultWeaponAsset.Object;
	}
}

void ANYGameModeTraining::BeginPlay()
{
	Super::BeginPlay();

	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		GS->SetGamePhase(ENYGamePhase::Playing);
	}
}

void ANYGameModeTraining::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
	SetupTrainingPlayer(NewPlayer);
}

void ANYGameModeTraining::SetupTrainingPlayer(APlayerController* NewPlayer)
{
	if (!HasAuthority() || !NewPlayer)
	{
		return;
	}

	if (ANYPlayerStateStage* PS = NewPlayer->GetPlayerState<ANYPlayerStateStage>())
	{
		PS->SetPlayerPhase(ENYPlayerPhase::Alive);
	}

	ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(NewPlayer->GetPawn());
	if (!Character || !DefaultWeaponDefinition)
	{
		return;
	}

	if (UNYWeaponComponent* WeaponComp = Character->GetWeaponComponent())
	{
		WeaponComp->SetWeaponDefinition(DefaultWeaponDefinition);
	}
}
