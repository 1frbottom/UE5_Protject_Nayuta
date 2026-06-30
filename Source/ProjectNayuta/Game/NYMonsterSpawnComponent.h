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


// Spawn
public:
	void StartSpawning();
	void StopSpawning();

	void UpdateSpawnerData(
		TSubclassOf<ANYMonsterBase> NewMonsterClass,
		float NewInterval,
		int32 NewSpawnCountPerTick,
		float NewSpawnRadius);

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	TSubclassOf<ANYMonsterBase> MonsterClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpawnInterval = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpawnRadius = 500.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	int32 SpawnCountPerTick = 10;

private:
	FTimerHandle SpawnTimerHandle;

	void SpawnMonsterRoutine();

};
