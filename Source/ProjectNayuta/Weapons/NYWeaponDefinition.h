// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "NYWeaponDefinition.generated.h"

class ANYAttackPlayerBase;

/** Base weapon identity and class refs. Per-level tuning lives in DT_WeaponLevel. */
UCLASS(BlueprintType)
class PROJECTNAYUTA_API UNYWeaponDefinition : public UDataAsset
{
	GENERATED_BODY()

// Weapon
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FText DisplayName;

	/** Key into WeaponLevelDataTable rows (e.g. "Sword", "Axe"). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	FName WeaponID;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	TSubclassOf<ANYAttackPlayerBase> AttackClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float BaseDamage = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float AttackRange = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Weapon")
	float Cooldown = 1.0f;

};
