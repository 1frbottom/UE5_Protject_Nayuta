// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NYMonsterSpawnComponent.generated.h"

class ANYMonsterBase;
class ANYCharacterPlayer;

/** Server-only wave spawner; attach to ANYMonsterSpawner in the level. */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTNAYUTA_API UNYMonsterSpawnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNYMonsterSpawnComponent();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void StartSpawning();
	void StopSpawning();

	void UpdateSpawnerData(TSubclassOf<ANYMonsterBase> NewMonsterClass, float NewInterval);

protected:
	UPROPERTY(EditAnywhere, Category = "Spawn")
	TSubclassOf<ANYMonsterBase> MonsterClass;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnInterval = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	float SpawnRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Spawn")
	int32 SpawnCountPerTick = 100;

private:
	FTimerHandle SpawnTimerHandle;

	void SpawnMonsterRoutine();
};
