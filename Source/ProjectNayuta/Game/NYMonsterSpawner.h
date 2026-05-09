// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NYMonsterSpawner.generated.h"

class ANYMonsterBase;

UCLASS()
class PROJECTNAYUTA_API ANYMonsterSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ANYMonsterSpawner();

protected:
	virtual void BeginPlay() override;

public:
    void StartSpawning();
    void StopSpawning();

    void UpdateSpawnerData(TSubclassOf<class ANYMonsterBase> NewMonsterClass, float NewInterval);

protected:
    /* Must be designated in BP_MonsterSpawner */
    UPROPERTY(EditAnywhere, Category = "Spawn")
    TSubclassOf<ANYMonsterBase> MonsterClass;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnInterval;

    UPROPERTY(EditAnywhere, Category = "Spawn")
    float SpawnRadius;

private:
    FTimerHandle SpawnTimerHandle;

    void SpawnMonsterRoutine();


};
