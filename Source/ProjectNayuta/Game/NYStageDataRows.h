// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"

#include "NYStageDataRows.generated.h"



/** Row name = player level (e.g. "1", "2"). RequiredExp = EXP to reach the next level. */
USTRUCT(BlueprintType)
struct FNYPlayerLevelRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level")
	int32 RequiredExp = 100;
};

/** Row name = wave index (e.g. "1", "2"). MonsterType resolves via UNYStageContentRegistry. */
USTRUCT(BlueprintType)
struct FNYWaveDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 BaseTargetKillCnt = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FName MonsterType = TEXT("Melee");
};

/** Row name = monster reward id (e.g. "Default", "Melee"). Referenced by ANYMonsterBase::RewardRowID. */
USTRUCT(BlueprintType)
struct FNYMonsterRewardRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 ExpReward = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Reward")
	int32 GoldReward = 5;
};

/** Row name = "{WeaponID}_{Level}" (e.g. "Sword_1"). Lookup by WeaponID + Level columns. */
USTRUCT(BlueprintType)
struct FNYWeaponLevelRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponID;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float RangeMultiplier = 1.0f;

	/** Applied to Cooldown — lower values mean faster attacks. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float CooldownMultiplier = 1.0f;
};
