// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "NYStageContentRegistry.generated.h"

class ANYMonsterBase;
class UNYMonsterDefinition;

/** Resolves FName keys from scalar DataTables to UObject definitions. */
UCLASS(BlueprintType)
class PROJECTNAYUTA_API UNYStageContentRegistry : public UDataAsset
{
	GENERATED_BODY()

// Monster
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Monster")
	TArray<TObjectPtr<UNYMonsterDefinition>> MonsterDefinitions;

	const UNYMonsterDefinition* FindMonsterDefinition(FName MonsterType) const;

	UFUNCTION(BlueprintPure, Category = "Monster")
	TSubclassOf<ANYMonsterBase> ResolveMonsterClass(FName MonsterType) const;

};
