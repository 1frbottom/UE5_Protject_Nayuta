// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "NYMonsterDefinition.generated.h"

class ANYMonsterBase;

/**
 * Editor-authored monster type. Referenced from DT_WaveData via MonsterType (FName).
 */
UCLASS(BlueprintType)
class PROJECTNAYUTA_API UNYMonsterDefinition : public UDataAsset
{
	GENERATED_BODY()

// Monster
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
	FName MonsterType;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
	TSubclassOf<ANYMonsterBase> MonsterClass;

	/** When set, overrides ANYMonsterBase::RewardRowID for kill rewards. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
	FName RewardRowIDOverride;

};
