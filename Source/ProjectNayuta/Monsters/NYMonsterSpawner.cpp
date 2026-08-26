// Fill out your copyright notice in the Description page of Project Settings.

#include "Monsters/NYMonsterSpawner.h"

#include "Monsters/NYMonsterSpawnComponent.h"

ANYMonsterSpawner::ANYMonsterSpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnComponent = CreateDefaultSubobject<UNYMonsterSpawnComponent>(TEXT("SpawnComponent"));
}