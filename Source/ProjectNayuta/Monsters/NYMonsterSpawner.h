// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NYMonsterSpawner.generated.h"

class ANYMonsterBase;
class UNYMonsterSpawnComponent;

/**
 * Level-placed spawn point. Spawn logic lives on UNYMonsterSpawnComponent.
 * Keeps BP_MonsterSpawner and editor placement working without duplicating logic on AActor.
 */
UCLASS()
class PROJECTNAYUTA_API ANYMonsterSpawner : public AActor
{
	GENERATED_BODY()

public:
	ANYMonsterSpawner();


// Spawn
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawn")
	TObjectPtr<UNYMonsterSpawnComponent> SpawnComponent;

};
