// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Engine/DataTable.h"

#include "NYGameMode.generated.h"



class ANYPlayerControllerStage;

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
class PROJECTNAYUTA_API ANYGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANYGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
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
	UPROPERTY()
	int32 AlivePlayerCnt = 0;

	void StartNextWave();
	void GameOver();


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


// Spawner
public:
	void RegisterSpawner(class ANYMonsterSpawner* Spawner);

protected:
	UPROPERTY(Transient)
	TArray<ANYMonsterSpawner*> ActiveSpawners;

	void SetSpawnersActive(bool bIsActive);



};
