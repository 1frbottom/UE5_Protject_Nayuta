// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameModeBase.h"
#include "Monsters/NYMonsterLifecycleInterface.h"
#include "NYGameModeTraining.generated.h"



class ANYMonsterBase;
class UNYWeaponDefinition;

UENUM(BlueprintType)
enum class ENYTrainingMonsterMode : uint8
{
	Idle UMETA(DisplayName = "Idle"),
	Chase UMETA(DisplayName = "Chase"),
};

/**
 * Standalone weapon / combat sandbox. Reuses Stage PS/Pawn/GS without wave/reward loop.
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameModeTraining : public ANYGameModeBase, public INYMonsterLifecycleInterface
{
	GENERATED_BODY()

public:
	ANYGameModeTraining();

protected:
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

// Training
public:
	/** Server: destroy previous test monster (if any) and spawn MonsterClass at TrainingSpawnTransform. */
	void SpawnTrainingMonster(TSubclassOf<ANYMonsterBase> MonsterClass, APlayerController* RequestingPC);

	/** Server: Idle = visible no seek; Chase = seek (+ existing melee/ranged attack timers). */
	void SetTrainingMonsterMode(ENYTrainingMonsterMode NewMode, APlayerController* RequestingPC);

	/** Server: snap back to TrainingSpawnTransform, full HP, re-apply current mode. */
	void ResetTrainingMonster(APlayerController* RequestingPC);

	/** Server: set MaxHP + refill CurrHp on the requesting player's PlayerState. */
	void SetTrainingPlayerMaxHp(APlayerController* RequestingPC, float NewMaxHp);

	/** Server: set MaxHp on active training monster (and remember for respawn/reset). */
	void SetTrainingMonsterMaxHp(float NewMaxHp);

	float GetTrainingMonsterMaxHp() const;
	float GetTrainingMonsterCurrHp() const;

protected:
	/** Granted to each player pawn on spawn when set. */
	UPROPERTY(EditDefaultsOnly, Category = "Training")
	TObjectPtr<UNYWeaponDefinition> DefaultWeaponDefinition;

	/** Where the training monster appears (set on BP_GameMode_Training or level override). */
	UPROPERTY(EditAnywhere, Category = "Training")
	FTransform TrainingSpawnTransform;

	/** Server: Playing phase + Alive + optional default weapon. */
	void SetupTrainingPlayer(APlayerController* NewPlayer);

	/** Server: apply CurrentTrainingMode to ActiveTrainingMonster. */
	void ApplyTrainingMode(APlayerController* RequestingPC);

	/** Server: apply TrainingMonsterMaxHpOverride if set (> 0). */
	void ApplyMonsterMaxHpOverride();

	UPROPERTY(Transient)
	TObjectPtr<ANYMonsterBase> ActiveTrainingMonster;

	UPROPERTY(Transient)
	TSubclassOf<ANYMonsterBase> LastTrainingMonsterClass;

	UPROPERTY(Transient)
	ENYTrainingMonsterMode CurrentTrainingMode = ENYTrainingMonsterMode::Idle;

	/** 0 = use monster class default MaxHp. */
	UPROPERTY(Transient)
	float TrainingMonsterMaxHpOverride = 0.0f;


// MonsterLifecycle
public:
	/**
	 * Server: the sandbox monster is reused instead of pooled, so a defeat resets it in place.
	 * False for anything that is not the active test monster, which lets it destroy itself.
	 */
	virtual bool ReclaimMonster(ANYMonsterBase* Monster) override;
};
