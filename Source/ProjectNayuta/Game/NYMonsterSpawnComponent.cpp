// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYMonsterSpawnComponent.h"

#include "Math/UnrealMathUtility.h"
#include "NavigationSystem.h"

#include "Game/NYGameModeStage.h"
#include "Game/NYGameStateBase.h"
#include "Game/NYMonsterPoolComponent.h"

#include "Player/NYPlayerStateStage.h"

#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"

namespace
{
	/** Tight XY so the point stays near the rolled location; tall Z covers steep height gaps. */
	const FVector NavProjectExtent(100.0f, 100.0f, 5000.0f);
	constexpr int32 NavProjectAttempts = 3;

	bool TryProjectSpawnLocation(UNavigationSystemV1* NavSys, const FVector& Candidate, FVector& OutLocation)
	{
		FNavLocation NavLoc;
		if (!NavSys->ProjectPointToNavigation(Candidate, NavLoc, NavProjectExtent))
		{
			return false;
		}

		OutLocation = NavLoc.Location;
		return true;
	}
}

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

// Spawn
void UNYMonsterSpawnComponent::StartSpawning()
{
	AActor* Owner = GetOwner();
	if (!Owner || !Owner->HasAuthority() || !MonsterClass)
	{
		return;
	}

	if (UWorld* World = GetWorld())
	{
		if (!FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
		{
			UE_LOG(LogTemp, Warning, TEXT("NYMonsterSpawnComponent: no NavigationSystem; wave spawn requires a built NavMesh."));
		}

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

void UNYMonsterSpawnComponent::UpdateSpawnerData(
	TSubclassOf<ANYMonsterBase> NewMonsterClass,
	float NewInterval,
	int32 NewSpawnCountPerTick,
	float NewSpawnRadius)
{
	if (NewMonsterClass)
	{
		MonsterClass = NewMonsterClass;
	}

	SpawnInterval = FMath::Max(NewInterval, KINDA_SMALL_NUMBER);
	SpawnCountPerTick = FMath::Max(NewSpawnCountPerTick, 1);
	SpawnRadius = FMath::Max(NewSpawnRadius, 0.0f);

	if (SpawnTimerHandle.IsValid())
	{
		StopSpawning();
		StartSpawning();
	}
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

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (!NavSys)
	{
		return;
	}

	// Batch pull from GameMode pool and activate around a random alive player.
	for (int32 i = 0; i < SpawnCountPerTick; ++i)
	{
		FVector SpawnLocation = FVector::ZeroVector;
		bool bFoundNavLocation = false;

		for (int32 Attempt = 0; Attempt < NavProjectAttempts; ++Attempt)
		{
			const FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
			FVector Candidate = TargetCharacter->GetActorLocation() + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
			Candidate.X += FMath::RandRange(-50.0f, 50.0f) * i;
			Candidate.Y += FMath::RandRange(-50.0f, 50.0f) * i;

			if (TryProjectSpawnLocation(NavSys, Candidate, SpawnLocation))
			{
				bFoundNavLocation = true;
				break;
			}
		}

		if (!bFoundNavLocation)
		{
			continue;
		}

		ANYMonsterBase* SpawnedMonster = Pool->GetMonster(SpawnLocation, FRotator::ZeroRotator);
		if (SpawnedMonster)
		{
			SpawnedMonster->ActivateOnServer(TargetCharacter, SpawnLocation);
		}
	}
}
