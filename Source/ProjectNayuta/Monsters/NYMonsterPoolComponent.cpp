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

	// Reset() only drops pointers. Destroy the previous class or the actors stay in the world.
	DestroyInactiveMonsters();
	InactivePool.Reset(PoolSize);
	PooledMonsterClass = MonsterClass;

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

	if (PooledMonsterClass && Monster->GetClass() != PooledMonsterClass)
	{
		Monster->Destroy();
		return;
	}

	Monster->DeactivateOnServer();
	InactivePool.Add(Monster);
}

void UNYMonsterPoolComponent::DestroyInactiveMonsters()
{
	for (ANYMonsterBase* Monster : InactivePool)
	{
		if (!IsValid(Monster))
		{
			continue;
		}

		// DormantAll hides these from net relevancy; wake so Destroy replicates to clients.
		Monster->SetNetDormancy(DORM_Awake);
		Monster->Destroy();
	}

	InactivePool.Reset();
}
