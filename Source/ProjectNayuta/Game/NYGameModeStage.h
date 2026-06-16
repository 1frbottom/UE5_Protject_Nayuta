// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameModeBase.h"
#include "Engine/DataTable.h"

#include "Game/NYStageDataRows.h"
#include "Player/NYPlayerStateStage.h"

#include "NYGameModeStage.generated.h"



class ANYPlayerControllerStage;
class ANYMonsterBase;
class UNYMonsterPoolComponent;
class UNYMonsterSpawnComponent;
class UNYStageContentRegistry;

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameModeStage : public ANYGameModeBase
{
	GENERATED_BODY()
	
public:
	ANYGameModeStage();

	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;

	// Wave
public:
	UPROPERTY(EditDefaultsOnly, Category = "Wave")
	TObjectPtr<UDataTable> WaveDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> PlayerLevelDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> MonsterRewardDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UNYStageContentRegistry> StageContentRegistry;

	/** EXP required to advance from InLevel. Returns 0 when the row is missing (max level). */
	int32 GetRequiredExpForLevel(int32 InLevel) const;

	/** Lookup monster kill rewards by RewardRowID. Returns false when the row is missing. */
	bool TryGetMonsterRewards(FName RewardRowID, int32& OutExp, int32& OutGold) const;

	/** Highest Level row for WeaponID in WeaponLevelDataTable. Returns 1 when no rows exist. */
	int32 GetMaxWeaponLevel(FName WeaponID) const;

	/** Lookup per-level multipliers by WeaponID + Level. Returns false when the row is missing. */
	bool TryGetWeaponLevelRow(FName WeaponID, int32 Level, FNYWeaponLevelRow& OutRow) const;

	int32 CurrWave = 0;
	int32 CurrKillCnt = 0;
	int32 TargetKillCnt = 0;

	// Server: kill count + reward grant. Killer may be null (rewards all alive players).
	void OnEnemyKilled(class AController* KillerController, class ANYMonsterBase* KilledMonster);
	void OnPlayerDied(ANYPlayerControllerStage* PC_victim);

protected:
	void StartNextWave();
	void TryStartFirstWave();
	void GameOver();

	/** PlayerArray queries — single source of truth instead of manual ++/-- counters. */
	int32 CountPlayersWithPawn() const;
	int32 CountPlayersInPhase(ENYPlayerPhase Phase) const;


	// Reward
public:
	int32 RewardedPlayerCnt = 0;
	FTimerHandle RewardTimeoutHandle;

	void OnPlayerRewarded();

protected:
	void StartRewardPhase();



	// Retry
public:
	void AddRetryVote();

protected:
	int32 RetryVoteCount = 0;


	// Spawner / pool (server-only via GameMode)
public:
	FORCEINLINE UNYMonsterPoolComponent* GetMonsterPoolComponent() const { return MonsterPoolComponent; }

	void RegisterSpawnComponent(UNYMonsterSpawnComponent* SpawnComponent);


protected:
	/** Default subobject; pre-warms monsters on BeginPlay (server only). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<UNYMonsterPoolComponent> MonsterPoolComponent;

	UPROPERTY(EditDefaultsOnly, Category = "Spawner")
	int32 InitialPoolSize = 100;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNYMonsterSpawnComponent>> ActiveSpawnComponents;

	void SetSpawnersActive(bool bIsActive);

	TSubclassOf<ANYMonsterBase> ResolveMonsterClass(FName MonsterType) const;
	void EnsureMonsterPoolForClass(TSubclassOf<ANYMonsterBase> MonsterClass);
	void ApplyWaveDataToSpawners(const FNYWaveDataRow& WaveData);

	/** Server: apply monster Exp/Gold to killer, or all alive players if killer is invalid. */
	void GrantKillRewards(AController* KillerController, int32 ExpAmount, int32 GoldAmount);

	UPROPERTY(Transient)
	TSubclassOf<ANYMonsterBase> CachedPoolMonsterClass;



};
