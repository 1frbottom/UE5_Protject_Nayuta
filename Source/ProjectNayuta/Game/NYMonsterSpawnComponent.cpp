// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYMonsterSpawnComponent.h"

#include "Math/UnrealMathUtility.h"

#include "Game/NYGameModeStage.h"
#include "Game/NYGameStateBase.h"
#include "Game/NYMonsterPoolComponent.h"

#include "Player/NYPlayerStateStage.h"

#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"

UNYMonsterSpawnComponent::UNYMonsterSpawnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UNYMonsterSpawnComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority())
	{
		return;
	}

	// Register even when MonsterClass is unset; wave data applies via UpdateSpawnerData().
	if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
	{
		GM->RegisterSpawnComponent(this);
	}
}

void UNYMonsterSpawnComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopSpawning();
	Super::EndPlay(EndPlayReason);
}

void UNYMonsterSpawnComponent::StartSpawning()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !MonsterClass)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			SpawnTimerHandle, this, &UNYMonsterSpawnComponent::SpawnMonsterRoutine, SpawnInterval, true);
	}
}

void UNYMonsterSpawnComponent::StopSpawning()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(SpawnTimerHandle);
	}
}

void UNYMonsterSpawnComponent::UpdateSpawnerData(TSubclassOf<ANYMonsterBase> NewMonsterClass, float NewInterval)
{
	if (NewMonsterClass)
	{
		MonsterClass = NewMonsterClass;
	}

	SpawnInterval = NewInterval;
}

void UNYMonsterSpawnComponent::SpawnMonsterRoutine()
{
	TArray<ANYCharacterPlayer*> AlivePlayers;

	if (ANYGameStateBase* GS = GetWorld()->GetGameState<ANYGameStateBase>())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			ANYPlayerStateStage* MyPS = Cast<ANYPlayerStateStage>(PS);
			if (!MyPS || MyPS->GetPlayerPhase() != ENYPlayerPhase::Alive)
			{
				continue;
			}

			if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(MyPS->GetPawn()))
			{
				AlivePlayers.Add(Character);
			}
		}
	}

	if (AlivePlayers.IsEmpty())
	{
		return;
	}

	ANYCharacterPlayer* TargetCharacter = AlivePlayers[FMath::RandRange(0, AlivePlayers.Num() - 1)];

	ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode());
	if (!GM || !GM->GetMonsterPoolComponent())
	{
		return;
	}

	UNYMonsterPoolComponent* Pool = GM->GetMonsterPoolComponent();

	// Batch pull from GameMode pool and activate around a random alive player.
	for (int32 i = 0; i < SpawnCountPerTick; ++i)
	{
		const FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
		FVector SpawnLocation = TargetCharacter->GetActorLocation() + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);

		SpawnLocation.X += FMath::RandRange(-50.0f, 50.0f) * i;
		SpawnLocation.Y += FMath::RandRange(-50.0f, 50.0f) * i;
		SpawnLocation.Z += 50.0f;

		ANYMonsterBase* SpawnedMonster = Pool->GetMonster(SpawnLocation, FRotator::ZeroRotator);
		if (SpawnedMonster)
		{
			SpawnedMonster->ActivateOnServer(TargetCharacter, SpawnLocation);
		}
	}
}
