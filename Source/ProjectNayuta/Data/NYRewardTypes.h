// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "NYRewardTypes.generated.h"



class UNYWeaponDefinition;

// RewardType
UENUM(BlueprintType)
enum class ENYRewardType : uint8
{
	StatMaxHp,
	StatMoveSpeed,
	NewWeapon,
	WeaponUpgrade,
};

UENUM(BlueprintType)
enum class ENYRewardWeaponSlot : uint8
{
	None,
	Primary,
	Secondary,
};

// RewardOffer
/** Replicated to owning client for WBP_Reward display. Server applies via RewardType + payload fields. */
USTRUCT(BlueprintType)
struct FNYRewardOffer
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	int32 SlotIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	ENYRewardType RewardType = ENYRewardType::StatMaxHp;

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	FText Description;

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	float StatValue = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	TObjectPtr<UNYWeaponDefinition> WeaponDefinition = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = "Reward")
	ENYRewardWeaponSlot WeaponSlot = ENYRewardWeaponSlot::None;
};
