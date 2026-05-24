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
	{
		// 풀이 다 떨어졌을 때의 예외 처리 (새로 스폰하거나 널 반환)
		// 당장 내일 테스트용이므로 널 반환으로 둡니다. 필요시 동적 할당 추가.

		return nullptr;
	}

	// 배열 맨 뒤에서 하나 꺼내기
	ANYMonsterBase* Monster = MonsterPool.Pop();
	if (Monster)
	{
		Monster->SetActorLocationAndRotation(SpawnLocation, SpawnRotation);
		Monster->SetActorHiddenInGame(false);
		Monster->SetActorEnableCollision(true);
		Monster->SetActorTickEnabled(true);

		// TODO: 여기서 몬스터 HP 초기화 등 리셋 로직 호출 필요


	}

	return Monster;
}

void ANYMonsterPoolManager::ReturnMonster(ANYMonsterBase* Monster)
{
	if (!Monster)
		return;

	Monster->SetActorHiddenInGame(true);
	Monster->SetActorEnableCollision(false);
	Monster->SetActorTickEnabled(false);

	MonsterPool.Add(Monster);
}

