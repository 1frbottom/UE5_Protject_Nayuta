// Fill out your copyright notice in the Description page of Project Settings.

#include "Game/NYGameMode.h"

#include "Kismet/GameplayStatics.h"

#include "Player/NYPlayerControllerStage.h"
#include "Player/NYPlayerStateBase.h"

#include "Game/NYGameStateBase.h"
#include "Game/NYMonsterSpawner.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"



ANYGameMode::ANYGameMode()
{



}

void ANYGameMode::BeginPlay()
{
	Super::BeginPlay();

	// GS->SetGamePhase(ENYGamePhase::Waiting);
	

	// TODO : 멀티플레이 동기화를 위해 일정 시간 대기 후 시작하도록 수정 필요
	

	StartNextWave();
}

void ANYGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	AlivePlayerCnt++;

}

void ANYGameMode::OnEnemyKilled()
{
	CurrKillCnt++;

	// TODO : 경험치, 골드
	

	if (CurrKillCnt >= TargetKillCnt)
	{
		StartRewardPhase();
	}
}

void ANYGameMode::StartNextWave()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("StartNextWave called! wave : %d"), CurrWave + 1));

	CurrWave++;
	CurrKillCnt = 0;

	if (WaveDataTable)
	{
		FName RowName = FName(*FString::FromInt(CurrWave));
		FNYWaveDataRow* WaveData = WaveDataTable->FindRow<FNYWaveDataRow>(RowName, TEXT("WaveContext"));

		if (WaveData)
		{
			// Scale by number of players
			int32 Multiplier = FMath::Max(1, AlivePlayerCnt);
			TargetKillCnt = (WaveData->BaseTargetKillCnt) * Multiplier;

			for (ANYMonsterSpawner* Spawner : ActiveSpawners)
				if (Spawner)
				{
					Spawner->UpdateSpawnerData(WaveData->MonsterClass, WaveData->SpawnInterval);
				}
		}
		else
		{
			// TODO : GameClear();

			// debug
			GEngine->AddOnScreenDebugMessage(
				-1, 5.f, FColor::Red,
				FString::Printf(TEXT("GAME CLEAR!!!"))
			);

			UE_LOG(LogTemp, Error, TEXT("[DEBUG] GAME CLEAR!!! DataTable End Reached."));

			return;
		}
	}

	if (ANYGameStateBase* GS = GetGameState<ANYGameStateBase>())
	{
		GS->SetGamePhase(ENYGamePhase::Playing);
		GS->ReplicatedTargetKillCnt = TargetKillCnt;
		GS->ReplicatedKillCnt = 0;

		for (APlayerState* PS : GS->PlayerArray)
		{
			if (ANYPlayerStateBase* MyPS = Cast<ANYPlayerStateBase>(PS))
			{
				MyPS->SetPlayerState(ENYPlayerState::Alive);

				// TODO : revive


			}
		}
	}

	SetSpawnersActive(true);


}

void ANYGameMode::OnPlayerDied(ANYPlayerControllerStage* PC_victim)
{
	AlivePlayerCnt--;

	// PC_victim->UnPossess();

	if (AlivePlayerCnt <= 0)
	{
		GameOver();
	}
}

void ANYGameMode::GameOver()
{
	// update Game Phase 
	ANYGameStateBase* GS = GetGameState<ANYGameStateBase>();
	if (GS)
		GS->SetGamePhase(ENYGamePhase::GameOver);

	SetSpawnersActive(false);


}

void ANYGameMode::StartRewardPhase()
{
	SetSpawnersActive(false);
	
	// TODO : 게임을 멈추던지, 무적으로 만들던지, 진행 멈춰야함, 일단 아래처럼 해놓긴 했는데 뭔가 별로같음
	TArray<AActor*> AliveMonsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANYMonsterBase::StaticClass(), AliveMonsters);
	for (AActor* Actor : AliveMonsters)
		if (ANYMonsterBase* Monster = Cast<ANYMonsterBase>(Actor))
		{
			Monster->Destroy(); // 혹은 체력을 0으로 깎는 함수 호출
		}

	// GameState
	ANYGameStateBase* GS = GetGameState<ANYGameStateBase>();
	if (GS)
	{
		GS->SetGamePhase(ENYGamePhase::Rewarding);

		// PlayerStates
		for (APlayerState* PS : GS->PlayerArray)
		{
			ANYPlayerStateBase* MyPS = Cast<ANYPlayerStateBase>(PS);
			if (MyPS)
			{
				MyPS->SetPlayerState(ENYPlayerState::Rewarding);
			}
		}
	}

	

	RewardedPlayerCnt = 0;

	// TODO : 보상 선택 타임아웃 타이머


}

void ANYGameMode::OnPlayerRewarded()
{
	// TODO : 플레이어 선택 받아서 적용

	RewardedPlayerCnt++;
	if (RewardedPlayerCnt >= GetNumPlayers())
	{
		StartNextWave();
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("reward clicked! current : %d / needed : %d"), RewardedPlayerCnt, GetNumPlayers()));
}

void ANYGameMode::AddRetryVote()
{
	RetryVoteCount++;

	// TODO : GameState에 현재 투표수 동기화, UI update
	if (ANYGameStateBase* GS = GetGameState<ANYGameStateBase>())
	{


	}

	if (RetryVoteCount >= GetNumPlayers())
	{
		GetWorld()->ServerTravel("?Restart", false);
	}


}

void ANYGameMode::RegisterSpawner(ANYMonsterSpawner* Spawner)
{
	if (Spawner && !ActiveSpawners.Contains(Spawner))
	{
		ActiveSpawners.Add(Spawner);
	}
}

void ANYGameMode::SetSpawnersActive(bool bActive)
{
	for (ANYMonsterSpawner* Spawner : ActiveSpawners)
		if (Spawner)
			if (bActive)
				Spawner->StartSpawning();
			else
				Spawner->StopSpawning();


}