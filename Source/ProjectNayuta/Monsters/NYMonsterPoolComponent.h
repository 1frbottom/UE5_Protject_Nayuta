// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NYMonsterPoolComponent.generated.h"



class ANYMonsterBase;

/**
 * Server-only monster object pool. Lives on ANYGameModeStage as a default subobject.
 * GameMode is authority-only, so this component is never replicated to clients.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTNAYUTA_API UNYMonsterPoolComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNYMonsterPoolComponent();


// Pool
public:
	/** Pre-spawns PoolSize monsters, hides them, and stores them in InactivePool. */
	void InitializePool(TSubclassOf<ANYMonsterBase> MonsterClass, int32 PoolSize);

	/** Takes a monster from the pool. SpawnLocation/Rotation are applied later in ActivateOnServer. */
	ANYMonsterBase* GetMonster(FVector SpawnLocation, FRotator SpawnRotation);

	/** Deactivates the monster and returns it to InactivePool. */
	void ReturnMonster(ANYMonsterBase* Monster);

protected:
	/** Monsters waiting to be activated (LIFO via Pop). */
	UPROPERTY()
	TArray<TObjectPtr<ANYMonsterBase>> InactivePool;

	/** Class last passed to InitializePool. ReturnMonster destroys mismatches instead of pooling them. */
	UPROPERTY(Transient)
	TSubclassOf<ANYMonsterBase> PooledMonsterClass;

private:
	/** Destroys every actor still referenced by InactivePool, then empties the array. */
	void DestroyInactiveMonsters();

};
