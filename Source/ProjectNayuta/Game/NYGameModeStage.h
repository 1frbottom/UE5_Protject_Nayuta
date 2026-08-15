// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameModeBase.h"
#include "Engine/DataTable.h"

#include "Game/NYStageDataRows.h"
#include "Game/NYRewardTypes.h"
#include "Game/NYMonsterLifecycleInterface.h"
#include "Player/NYPlayerStateStage.h"

#include "NYGameModeStage.generated.h"



class ANYPlayerControllerStage;
class ANYMonsterBase;
class ANYCharacterPlayer;
class UNYMonsterPoolComponent;
class UNYMonsterSpawnComponent;
class UNYStageContentRegistry;
class UNYWeaponComponent;
class UNYWeaponDefinition;

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameModeStage : public ANYGameModeBase, public INYMonsterLifecycleInterface
{
	GENERATED_BODY()

public:
	ANYGameModeStage();

protected:
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
	TObjectPtr<UDataTable> RewardPoolDataTable;

	/** Number of choices offered per alive player each reward phase. */
	UPROPERTY(EditDefaultsOnly, Category = "Reward", meta = (ClampMin = "1"))
	int32 RewardOfferCount = 3;

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

	void OnPlayerDied(ANYPlayerControllerStage* PC_victim);

protected:
	void StartNextWave();
	void TryStartFirstWave();
	void GameOver();
	void GameClear();

	/** PlayerArray queries — single source of truth instead of manual ++/-- counters. */
	int32 CountPlayersWithPawn() const;
	int32 CountPlayersInPhase(ENYPlayerPhase Phase) const;


// Reward
public:
	int32 RewardedPlayerCnt = 0;
	FTimerHandle RewardTimeoutHandle;

	void OnPlayerRewarded();

	/** Server: advance wave when every connected player is Ready. */
	void TryAdvanceWaveAfterRewards();

protected:
	void StartRewardPhase();

	TArray<FNYRewardOffer> GenerateRewardOffers(ANYPlayerStateStage* PlayerState) const;


// Retry
public:
	void AddRetryVote();

protected:
	int32 RetryVoteCount = 0;


// GameClear
public:
	/** Server: travel all players back to the main menu map. */
	void ReturnToMainMenu();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "GameClear")
	FString MainMenuMapPath = TEXT("/Game/Levels/LV_MainMenu");


// Spawner
public:
	FORCEINLINE UNYMonsterPoolComponent* GetMonsterPoolComponent() const { return MonsterPoolComponent; }

	void RegisterSpawnComponent(UNYMonsterSpawnComponent* SpawnComponent);

protected:
	/** Default subobject; pre-warms monsters on BeginPlay (server only). */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<UNYMonsterPoolComponent> MonsterPoolComponent;

	/** Pre-warmed monster count; sized for peak concurrent spawns across all spawners. */
	UPROPERTY(EditDefaultsOnly, Category = "Spawner", meta = (ClampMin = "1"))
	int32 InitialPoolSize = 500;

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


// MonsterLifecycle
public:
	/** Server: kill count + reward grant. Killer may be null (rewards all alive players). */
	virtual void NotifyMonsterKilled(AController* KillerController, ANYMonsterBase* Monster) override;

	/** Server: hand the corpse back to the pool. False when no pool exists, so the caller destroys it. */
	virtual bool ReclaimMonster(ANYMonsterBase* Monster) override;

};
