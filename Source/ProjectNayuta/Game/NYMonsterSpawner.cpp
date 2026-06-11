// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYMonsterSpawner.h"

#include "Game/NYMonsterSpawnComponent.h"

ANYMonsterSpawner::ANYMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnComponent = CreateDefaultSubobject<UNYMonsterSpawnComponent>(TEXT("SpawnComponent"));
}