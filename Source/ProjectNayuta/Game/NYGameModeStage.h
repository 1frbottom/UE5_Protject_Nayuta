// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameModeBase.h"
#include "Engine/DataTable.h"

#include "Player/NYPlayerStateStage.h"

#include "NYGameModeStage.generated.h"



class ANYPlayerControllerStage;
class UNYMonsterPoolComponent;
class UNYMonsterSpawnComponent;

USTRUCT(BlueprintType)
struct FNYWaveDataRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 BaseTargetKillCnt = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SpawnInterval = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<class ANYMonsterBase> MonsterClass;
};

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

	int32 CurrWave = 0;
	int32 CurrKillCnt = 0;
	int32 TargetKillCnt = 0;

	void OnEnemyKilled();
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
	int32 InitialPoolSize = 1000;
	
	UPROPERTY(Transient)
	TArray<TObjectPtr<UNYMonsterSpawnComponent>> ActiveSpawnComponents;

	void SetSpawnersActive(bool bIsActive);



};
