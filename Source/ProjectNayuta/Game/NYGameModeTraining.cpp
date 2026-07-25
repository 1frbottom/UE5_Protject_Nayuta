// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYGameModeTraining.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Game/NYGameStateStage.h"
#include "Player/NYPlayerControllerTraining.h"
#include "Player/NYPlayerStateStage.h"
#include "Weapons/NYWeaponComponent.h"
#include "Weapons/NYWeaponDefinition.h"

ANYGameModeTraining::ANYGameModeTraining()
{
	// Local sandbox — no lobby/session travel.
	bUseSeamlessTravel = false;

	TrainingSpawnTransform = FTransform(FRotator::ZeroRotator, FVector(500.0f, 0.0f, 100.0f));
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

void ANYGameModeTraining::SpawnTrainingMonster(TSubclassOf<ANYMonsterBase> MonsterClass, APlayerController* RequestingPC)
{
	if (!HasAuthority() || !MonsterClass)
	{
		return;
	}

	if (ActiveTrainingMonster)
	{
		ActiveTrainingMonster->Destroy();
		ActiveTrainingMonster = nullptr;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	ANYMonsterBase* SpawnedMonster = World->SpawnActor<ANYMonsterBase>(
		MonsterClass,
		TrainingSpawnTransform.GetLocation(),
		TrainingSpawnTransform.GetRotation().Rotator(),
		SpawnParams);

	if (!SpawnedMonster)
	{
		return;
	}

	ActiveTrainingMonster = SpawnedMonster;
	LastTrainingMonsterClass = MonsterClass;
	ApplyMonsterMaxHpOverride();
	ApplyTrainingMode(RequestingPC);
}

void ANYGameModeTraining::SetTrainingMonsterMode(ENYTrainingMonsterMode NewMode, APlayerController* RequestingPC)
{
	if (!HasAuthority())
	{
		return;
	}

	CurrentTrainingMode = NewMode;

	if (ActiveTrainingMonster)
	{
		ApplyTrainingMode(RequestingPC);
	}
}

void ANYGameModeTraining::ResetTrainingMonster(APlayerController* RequestingPC)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!ActiveTrainingMonster)
	{
		if (LastTrainingMonsterClass)
		{
			SpawnTrainingMonster(LastTrainingMonsterClass, RequestingPC);
		}
		return;
	}

	ApplyMonsterMaxHpOverride();
	ApplyTrainingMode(RequestingPC);
}

void ANYGameModeTraining::NotifyTrainingMonsterDefeated(ANYMonsterBase* Monster)
{
	if (!HasAuthority() || !Monster || Monster != ActiveTrainingMonster)
	{
		return;
	}

	APlayerController* PC = nullptr;
	if (UWorld* World = GetWorld())
	{
		PC = World->GetFirstPlayerController();
	}

	ResetTrainingMonster(PC);
}

void ANYGameModeTraining::SetTrainingPlayerMaxHp(APlayerController* RequestingPC, float NewMaxHp)
{
	if (!HasAuthority() || !RequestingPC)
	{
		return;
	}

	const float SanitizedMaxHp = FMath::Max(1.0f, FMath::RoundToFloat(NewMaxHp));

	if (ANYPlayerStateStage* PS = RequestingPC->GetPlayerState<ANYPlayerStateStage>())
	{
		PS->SetMaxHp(SanitizedMaxHp);
		PS->SetCurrHp(PS->GetMaxHp());
		if (PS->GetCurrHp() > 0.0f && PS->GetPlayerPhase() == ENYPlayerPhase::Dead)
		{
			PS->SetPlayerPhase(ENYPlayerPhase::Alive);
		}
	}
}

void ANYGameModeTraining::SetTrainingMonsterMaxHp(float NewMaxHp)
{
	if (!HasAuthority())
	{
		return;
	}

	TrainingMonsterMaxHpOverride = FMath::Max(1.0f, FMath::RoundToFloat(NewMaxHp));

	if (ActiveTrainingMonster)
	{
		ActiveTrainingMonster->SetMaxHpOnServer(TrainingMonsterMaxHpOverride);
	}
}

float ANYGameModeTraining::GetTrainingMonsterMaxHp() const
{
	if (ActiveTrainingMonster)
	{
		return ActiveTrainingMonster->GetMaxHp();
	}

	if (TrainingMonsterMaxHpOverride > 0.0f)
	{
		return TrainingMonsterMaxHpOverride;
	}

	if (LastTrainingMonsterClass)
	{
		if (const ANYMonsterBase* CDO = LastTrainingMonsterClass->GetDefaultObject<ANYMonsterBase>())
		{
			return CDO->GetMaxHp();
		}
	}

	return 100.0f;
}

float ANYGameModeTraining::GetTrainingMonsterCurrHp() const
{
	if (ActiveTrainingMonster)
	{
		return ActiveTrainingMonster->GetCurrentHp();
	}

	return GetTrainingMonsterMaxHp();
}

void ANYGameModeTraining::ApplyTrainingMode(APlayerController* RequestingPC)
{
	if (!HasAuthority() || !ActiveTrainingMonster)
	{
		return;
	}

	const FVector SpawnLocation = TrainingSpawnTransform.GetLocation();

	switch (CurrentTrainingMode)
	{
	case ENYTrainingMonsterMode::Idle:
		ActiveTrainingMonster->ActivateIdleOnServer(SpawnLocation);
		break;

	case ENYTrainingMonsterMode::Chase:
	{
		AActor* TargetActor = RequestingPC ? RequestingPC->GetPawn() : nullptr;
		ActiveTrainingMonster->ActivateOnServer(TargetActor, SpawnLocation);
		break;
	}
	}
}

void ANYGameModeTraining::ApplyMonsterMaxHpOverride()
{
	if (!HasAuthority() || !ActiveTrainingMonster || TrainingMonsterMaxHpOverride <= 0.0f)
	{
		return;
	}

	ActiveTrainingMonster->SetMaxHpOnServer(TrainingMonsterMaxHpOverride);
}
