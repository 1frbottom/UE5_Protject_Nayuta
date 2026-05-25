// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NYMonsterPoolManager.generated.h"



class ANYMonsterBase;

UCLASS()
class PROJECTNAYUTA_API ANYMonsterPoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ANYMonsterPoolManager();


public:
	void InitializePool(TSubclassOf<ANYMonsterBase> MonsterClass, int32 PoolSize);

	ANYMonsterBase* GetMonster(FVector SpawnLocation, FRotator SpawnRotation);
	void ReturnMonster(ANYMonsterBase* Monster);

protected:
	// 비활성화된 몬스터들을 담아둘 배열
	UPROPERTY()
	TArray<ANYMonsterBase*> MonsterPool;



};
