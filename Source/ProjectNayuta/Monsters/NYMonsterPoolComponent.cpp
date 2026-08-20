// Fill out your copyright notice in the Description page of Project Settings.

#include "Monsters/NYMonsterPoolComponent.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"

UNYMonsterPoolComponent::UNYMonsterPoolComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}


// Pool
void UNYMonsterPoolComponent::InitializePool(TSubclassOf<ANYMonsterBase> MonsterClass, int32 PoolSize)
{
	UWorld* World = GetWorld();
	if (!MonsterClass || !World)
	{
		return;
	}

	InactivePool.Reset(PoolSize);

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < PoolSize; ++i)
	{
		// Spawn at origin; real position is set when the spawner calls ActivateOnServer.
		ANYMonsterBase* SpawnedMonster = World->SpawnActor<ANYMonsterBase>(
			MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (SpawnedMonster)
		{
			// Same path a returned corpse takes, so a pre-warmed monster and a recycled one
			// start from identical hidden/dormant state.
			SpawnedMonster->DeactivateOnServer();
			InactivePool.Add(SpawnedMonster);
		}
	}
}

ANYMonsterBase* UNYMonsterPoolComponent::GetMonster(FVector SpawnLocation, FRotator SpawnRotation)
{
	(void)SpawnLocation;
	(void)SpawnRotation;

	if (InactivePool.IsEmpty())
	{
		return nullptr;
	}

	return InactivePool.Pop();
}

void UNYMonsterPoolComponent::ReturnMonster(ANYMonsterBase* Monster)
{
	if (!Monster || !Monster->IsActive())
	{
		return;
	}

	Monster->DeactivateOnServer();
	InactivePool.Add(Monster);
}
