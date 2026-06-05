// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYMonsterSpawner.h"

#include "Game/NYMonsterSpawnComponent.h"

ANYMonsterSpawner::ANYMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnComponent = CreateDefaultSubobject<UNYMonsterSpawnComponent>(TEXT("SpawnComponent"));
}

void ANYMonsterSpawner::StartSpawning()
{
	if (SpawnComponent)
	{
		SpawnComponent->StartSpawning();
	}
}

void ANYMonsterSpawner::StopSpawning()
{
	if (SpawnComponent)
	{
		SpawnComponent->StopSpawning();
	}
}

void ANYMonsterSpawner::UpdateSpawnerData(TSubclassOf<ANYMonsterBase> NewMonsterClass, float NewInterval)
{
	if (SpawnComponent)
	{
		SpawnComponent->UpdateSpawnerData(NewMonsterClass, NewInterval);
	}
}
