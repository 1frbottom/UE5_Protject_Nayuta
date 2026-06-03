// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYMonsterPoolManager.h"

#include "Characters\CharacterMonsters\NYMonsterBase.h"


ANYMonsterPoolManager::ANYMonsterPoolManager()
{



}

void ANYMonsterPoolManager::InitializePool(TSubclassOf<ANYMonsterBase> MonsterClass, int32 PoolSize)
{
	if (!MonsterClass || !GetWorld())
		return;

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	for (int32 i = 0; i < PoolSize; i++)
	{
		ANYMonsterBase* SpawnedMonster = GetWorld()->SpawnActor<ANYMonsterBase>(MonsterClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

		if (SpawnedMonster)
		{
			SpawnedMonster->SetActorHiddenInGame(true);
			SpawnedMonster->SetActorEnableCollision(false);
			SpawnedMonster->SetActorTickEnabled(false);
			MonsterPool.Add(SpawnedMonster);
		}
	}
}

ANYMonsterBase* ANYMonsterPoolManager::GetMonster(FVector SpawnLocation, FRotator SpawnRotation)
{

	if (MonsterPool.IsEmpty())
		return nullptr;

	return MonsterPool.Pop();
}

void ANYMonsterPoolManager::ReturnMonster(ANYMonsterBase* Monster)
{
	if (!Monster)
		return;

	Monster->DeactivateOnServer();
	MonsterPool.Add(Monster);
}

