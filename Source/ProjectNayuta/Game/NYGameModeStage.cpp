// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameModeStage.h"

#include "Kismet/GameplayStatics.h"

#include "Player/NYPlayerControllerStage.h"
#include "Player/NYPlayerStateStage.h"

#include "Game/NYGameStateStage.h"
#include "Game/NYMonsterSpawnComponent.h"
#include "Game/NYMonsterPoolComponent.h"
#include "Game/NYStageContentRegistry.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Game/NYGameStateStage.h"
#include "Weapons/NYWeaponLevelLibrary.h"



ANYGameModeStage::ANYGameModeStage()
{
	MonsterPoolComponent = CreateDefaultSubobject<UNYMonsterPoolComponent>(TEXT("MonsterPool"));
}

void ANYGameModeStage::BeginPlay()
{
	Super::BeginPlay();

	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		GS->SetGamePhase(ENYGamePhase::Waiting);
	}

	// First wave starts in TryStartFirstWave() after all connected players have pawns.

	// Pre-warm pool from wave 1 row (server-only; component lives on GameMode).
	if (MonsterPoolComponent && WaveDataTable)
	{
		const FNYWaveDataRow* FirstWaveData = WaveDataTable->FindRow<FNYWaveDataRow>(FName(TEXT("1")), TEXT("PoolInitContext"));
		if (FirstWaveData)
		{
			EnsureMonsterPoolForClass(ResolveMonsterClass(FirstWaveData->MonsterType));
		}
	}

	
}

void ANYGameModeStage::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (CurrWave == 0)
	{
		TryStartFirstWave();
	}
}

int32 ANYGameModeStage::CountPlayersWithPawn() const
{
	int32 Count = 0;

	if (const ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (PS && PS->GetPawn())
				Count++;
		}

	return Count;
}

int32 ANYGameModeStage::CountPlayersInPhase(ENYPlayerPhase Phase) const
{
	int32 Count = 0;

	if (const ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			if (const ANYPlayerStateStage* StagePS = Cast<ANYPlayerStateStage>(PS))
			{
				if (StagePS->GetPlayerPhase() == Phase)
				{
					Count++;
				}
			}
		}
	}

	return Count;
}

void ANYGameModeStage::TryStartFirstWave()
{
	if (CurrWave > 0)
	{
		return;
	}

	const int32 NumPlayers = GetNumPlayers();
	if (NumPlayers <= 0)
	{
		return;
	}

	if (CountPlayersWithPawn() < NumPlayers)
	{
		return;
	}

	StartNextWave();
}

void ANYGameModeStage::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (CurrWave == 0)
	{
		TryStartFirstWave();
	}

	if (GetNumPlayers() <= 0)
		return;

	ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	if (!GS)
		return;

	// Avoid reward-phase softlock when someone leaves during rewarding.
	if (GS->GetGamePhase() == ENYGamePhase::Rewarding)
	{
		if (RewardedPlayerCnt >= GetNumPlayers())
		{
			StartNextWave();
		}
	}

	// Avoid game-over retry softlock when someone leaves after voting.
	if (GS->GetGamePhase() == ENYGamePhase::GameOver)
	{
		if (RetryVoteCount >= GetNumPlayers())
		{
			GetWorld()->ServerTravel("?Restart", false);
		}
	}
}

void ANYGameModeStage::OnEnemyKilled(AController* KillerController, ANYMonsterBase* KilledMonster)
{
	CurrKillCnt++;

	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		GS->ReplicatedKillCnt = CurrKillCnt;
	}

	if (KilledMonster)
	{
		int32 ExpReward = 0;
		int32 GoldReward = 0;
		if (TryGetMonsterRewards(KilledMonster->GetRewardRowID(), ExpReward, GoldReward))
		{
			GrantKillRewards(KillerController, ExpReward, GoldReward);
		}
	}

	if (CurrKillCnt >= TargetKillCnt)
	{
		StartRewardPhase();
	}
}

int32 ANYGameModeStage::GetRequiredExpForLevel(int32 InLevel) const
{
	if (!PlayerLevelDataTable || InLevel <= 0)
	{
		return 0;
	}

	const FName RowName = FName(*FString::FromInt(InLevel));
	const FNYPlayerLevelRow* Row = PlayerLevelDataTable->FindRow<FNYPlayerLevelRow>(RowName, TEXT("PlayerLevel"));
	if (!Row || Row->RequiredExp <= 0)
	{
		return 0;
	}

	return Row->RequiredExp;
}

int32 ANYGameModeStage::GetMaxWeaponLevel(FName WeaponID) const
{
	const ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	return NYWeaponLevel::GetMaxLevel(GS ? GS->WeaponLevelDataTable : nullptr, WeaponID);
}

bool ANYGameModeStage::TryGetWeaponLevelRow(FName WeaponID, int32 Level, FNYWeaponLevelRow& OutRow) const
{
	const ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	return NYWeaponLevel::TryGetRow(GS ? GS->WeaponLevelDataTable : nullptr, WeaponID, Level, OutRow);
}

TSubclassOf<ANYMonsterBase> ANYGameModeStage::ResolveMonsterClass(FName MonsterType) const
{
	if (StageContentRegistry)
	{
		return StageContentRegistry->ResolveMonsterClass(MonsterType);
	}

	return nullptr;
}

void ANYGameModeStage::EnsureMonsterPoolForClass(TSubclassOf<ANYMonsterBase> MonsterClass)
{
	if (!MonsterPoolComponent || !MonsterClass || CachedPoolMonsterClass == MonsterClass)
	{
		return;
	}

	MonsterPoolComponent->InitializePool(MonsterClass, InitialPoolSize);
	CachedPoolMonsterClass = MonsterClass;
}

void ANYGameModeStage::ApplyWaveDataToSpawners(const FNYWaveDataRow& WaveData)
{
	const TSubclassOf<ANYMonsterBase> MonsterClass = ResolveMonsterClass(WaveData.MonsterType);
	if (!MonsterClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ANYGameModeStage] Unknown MonsterType: %s"), *WaveData.MonsterType.ToString());
		return;
	}

	EnsureMonsterPoolForClass(MonsterClass);

	for (UNYMonsterSpawnComponent* SpawnComponent : ActiveSpawnComponents)
	{
		if (SpawnComponent)
		{
			SpawnComponent->UpdateSpawnerData(MonsterClass, WaveData.SpawnInterval);
		}
	}
}

bool ANYGameModeStage::TryGetMonsterRewards(FName RewardRowID, int32& OutExp, int32& OutGold) const
{
	OutExp = 0;
	OutGold = 0;

	if (!MonsterRewardDataTable || RewardRowID.IsNone())
	{
		return false;
	}

	const FNYMonsterRewardRow* Row = MonsterRewardDataTable->FindRow<FNYMonsterRewardRow>(RewardRowID, TEXT("MonsterReward"));
	if (!Row)
	{
		return false;
	}

	OutExp = FMath::Max(0, Row->ExpReward);
	OutGold = FMath::Max(0, Row->GoldReward);
	return OutExp > 0 || OutGold > 0;
}

void ANYGameModeStage::GrantKillRewards(AController* KillerController, int32 ExpAmount, int32 GoldAmount)
{
	if (ExpAmount <= 0 && GoldAmount <= 0)
	{
		return;
	}

	auto GrantToPlayerState = [&](ANYPlayerStateStage* StagePS)
	{
		if (!StagePS || StagePS->GetPlayerPhase() != ENYPlayerPhase::Alive)
		{
			return;
		}

		if (ExpAmount > 0)
		{
			StagePS->AddExp(ExpAmount);
		}

		if (GoldAmount > 0)
		{
			StagePS->AddGold(GoldAmount);
		}
	};

	if (ANYPlayerStateStage* KillerPS = KillerController ? KillerController->GetPlayerState<ANYPlayerStateStage>() : nullptr)
	{
		GrantToPlayerState(KillerPS);
		return;
	}

	// No valid killer: share rewards with every alive player (co-op fallback).
	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		for (APlayerState* PS : GS->PlayerArray)
		{
			GrantToPlayerState(Cast<ANYPlayerStateStage>(PS));
		}
	}
}

void ANYGameModeStage::StartNextWave()
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Yellow, FString::Printf(TEXT("StartNextWave called! wave : %d"), CurrWave + 1));

	CurrWave++;
	CurrKillCnt = 0;

	// New run (wave 1): clear per-run exp/gold. Survives across waves; cleared on ServerTravel ?Restart.
	if (CurrWave == 1)
		if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
			for (APlayerState* PS : GS->PlayerArray)
			{
				if (ANYPlayerStateStage* StagePS = Cast<ANYPlayerStateStage>(PS))
					StagePS->ResetRunStats();
			}

	if (WaveDataTable)
	{
		FName RowName = FName(*FString::FromInt(CurrWave));
		FNYWaveDataRow* WaveData = WaveDataTable->FindRow<FNYWaveDataRow>(RowName, TEXT("WaveContext"));

		if (WaveData)
		{
			const int32 Multiplier = FMath::Max(1, GetNumPlayers());
			TargetKillCnt = WaveData->BaseTargetKillCnt * Multiplier;
			ApplyWaveDataToSpawners(*WaveData);
		}
		else
		{
			// TODO: GameClear();

			// debug
			GEngine->AddOnScreenDebugMessage(
				-1, 5.f, FColor::Red,
				FString::Printf(TEXT("GAME CLEAR!!!"))
			);

			UE_LOG(LogTemp, Error, TEXT("[DEBUG] GAME CLEAR!!! DataTable End Reached."));

			return;
		}
	}

	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		GS->SetGamePhase(ENYGamePhase::Playing);
		GS->ReplicatedTargetKillCnt = TargetKillCnt;
		GS->ReplicatedKillCnt = 0;

		for (APlayerState* PS : GS->PlayerArray)
		{
			if (ANYPlayerStateStage* MyPS = Cast<ANYPlayerStateStage>(PS))
			{
				if (MyPS->GetPawn() != nullptr)
				{
					MyPS->SetPlayerPhase(ENYPlayerPhase::Alive);
					MyPS->SetCurrHp(MyPS->GetMaxHp());
				}
			}
		}
	}

	SetSpawnersActive(true);


}

void ANYGameModeStage::OnPlayerDied(ANYPlayerControllerStage* PC_victim)
{
	(void)PC_victim;

	if (CountPlayersInPhase(ENYPlayerPhase::Alive) <= 0)
	{
		GameOver();
	}
}

void ANYGameModeStage::GameOver()
{
	// update Game Phase 
	ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	if (GS)
		GS->SetGamePhase(ENYGamePhase::GameOver);

	SetSpawnersActive(false);


}

void ANYGameModeStage::StartRewardPhase()
{
	SetSpawnersActive(false);

	// Return all active monsters to the pool (fallback: destroy if pool missing).
	TArray<AActor*> AliveMonsters;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ANYMonsterBase::StaticClass(), AliveMonsters);

	for (AActor* Actor : AliveMonsters)
		if (ANYMonsterBase* Monster = Cast<ANYMonsterBase>(Actor))
		{
			if (MonsterPoolComponent)
				MonsterPoolComponent->ReturnMonster(Monster);
			else
				Monster->Destroy();
		}

	// GameState
	ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	if (GS)
	{
		GS->SetGamePhase(ENYGamePhase::Rewarding);

		// PlayerStates
		for (APlayerState* PS : GS->PlayerArray)
		{
			ANYPlayerStateStage* MyPS = Cast<ANYPlayerStateStage>(PS);
			if (MyPS)
			{
				MyPS->SetPlayerPhase(ENYPlayerPhase::Rewarding);
			}
		}
	}


	RewardedPlayerCnt = 0;

	// TODO: Reward selection timeout timer.


}

void ANYGameModeStage::OnPlayerRewarded()
{
	// TODO: Apply the player's reward choice.

	RewardedPlayerCnt++;
	if (RewardedPlayerCnt >= GetNumPlayers())
	{
		StartNextWave();
	}

	GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Cyan, FString::Printf(TEXT("reward clicked! current : %d / needed : %d"), RewardedPlayerCnt, GetNumPlayers()));
}

void ANYGameModeStage::AddRetryVote()
{
	RetryVoteCount++;

	// TODO: Replicate retry vote count to GameState for UI.
	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{


	}

	if (RetryVoteCount >= GetNumPlayers())
	{
		GetWorld()->ServerTravel("?Restart", false);
	}


}

void ANYGameModeStage::RegisterSpawnComponent(UNYMonsterSpawnComponent* SpawnComponent)
{
	if (!SpawnComponent || ActiveSpawnComponents.Contains(SpawnComponent))
	{
		return;
	}

	ActiveSpawnComponents.Add(SpawnComponent);

	// GameMode BeginPlay can run StartNextWave before level spawners BeginPlay.
	if (ANYGameStateStage* GS = GetGameState<ANYGameStateStage>())
	{
		if (GS->GetGamePhase() == ENYGamePhase::Playing && WaveDataTable && CurrWave > 0)
		{
			const FName RowName = FName(*FString::FromInt(CurrWave));
			if (const FNYWaveDataRow* WaveData = WaveDataTable->FindRow<FNYWaveDataRow>(RowName, TEXT("LateSpawnRegister")))
			{
				ApplyWaveDataToSpawners(*WaveData);
			}

			SpawnComponent->StartSpawning();
		}
	}
}

void ANYGameModeStage::SetSpawnersActive(bool bActive)
{
	for (UNYMonsterSpawnComponent* SpawnComponent : ActiveSpawnComponents)
		if (SpawnComponent)
			if (bActive)
				SpawnComponent->StartSpawning();
			else
				SpawnComponent->StopSpawning();
}
