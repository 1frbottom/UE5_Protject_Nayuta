// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameModeStage.h"

#include "Kismet/GameplayStatics.h"

#include "Player/NYPlayerControllerStage.h"
#include "Player/NYPlayerStateStage.h"

#include "Game/NYGameStateStage.h"
#include "Game/NYMonsterSpawnComponent.h"
#include "Game/NYMonsterPoolComponent.h"
#include "Game/NYStageContentRegistry.h"
#include "Game/NYRewardTypes.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Weapons/NYWeaponComponent.h"
#include "Weapons/NYWeaponDefinition.h"
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
		TryAdvanceWaveAfterRewards();
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


// Wave
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

void ANYGameModeStage::OnPlayerDied(ANYPlayerControllerStage* PC_victim)
{
	(void)PC_victim;

	if (CountPlayersInPhase(ENYPlayerPhase::Alive) <= 0)
	{
		GameOver();
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
			GameClear();
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
				}
			}
		}
	}

	SetSpawnersActive(true);


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

void ANYGameModeStage::GameOver()
{
	// update Game Phase 
	ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	if (GS)
		GS->SetGamePhase(ENYGamePhase::GameOver);

	SetSpawnersActive(false);


}

void ANYGameModeStage::GameClear()
{
	// Server
	if (!HasAuthority())
	{
		return;
	}

	SetSpawnersActive(false);

	ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	if (GS)
	{
		GS->ReplicatedClearedWaveCount = FMath::Max(0, CurrWave - 1);
		GS->SetGamePhase(ENYGamePhase::GameClear);
	}
}

void ANYGameModeStage::ReturnToMainMenu()
{
	// Server
	if (!HasAuthority())
	{
		return;
	}

	GetWorld()->ServerTravel(MainMenuMapPath, false);
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


// Reward
void ANYGameModeStage::OnPlayerRewarded()
{
	TryAdvanceWaveAfterRewards();
}

void ANYGameModeStage::TryAdvanceWaveAfterRewards()
{
	if (GetNumPlayers() <= 0)
	{
		return;
	}

	if (CountPlayersInPhase(ENYPlayerPhase::Ready) >= GetNumPlayers())
	{
		StartNextWave();
	}
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

	ANYGameStateStage* GS = GetGameState<ANYGameStateStage>();
	if (!GS)
	{
		return;
	}

	GS->SetGamePhase(ENYGamePhase::Rewarding);

	RewardedPlayerCnt = 0;

	for (APlayerState* PS : GS->PlayerArray)
	{
		ANYPlayerStateStage* StagePS = Cast<ANYPlayerStateStage>(PS);
		if (!StagePS)
		{
			continue;
		}

		// Died before the wave ended — skip reward UI and count as ready.
		if (StagePS->GetPlayerPhase() == ENYPlayerPhase::Dead)
		{
			StagePS->SetPendingRewardOffers(TArray<FNYRewardOffer>());
			StagePS->SetPlayerPhase(ENYPlayerPhase::Ready);
			continue;
		}

		const TArray<FNYRewardOffer> Offers = GenerateRewardOffers(StagePS);
		if (Offers.Num() <= 0)
		{
			StagePS->SetPendingRewardOffers(TArray<FNYRewardOffer>());
			StagePS->SetPlayerPhase(ENYPlayerPhase::Ready);
			continue;
		}

		StagePS->SetPendingRewardOffers(Offers);
		StagePS->SetPlayerPhase(ENYPlayerPhase::Rewarding);
	}

	TryAdvanceWaveAfterRewards();

	// TODO: Reward selection timeout timer.
}

namespace NYRewardGeneration
{
	// before FNYRewardOffer ( no slotIndex )
	struct FCandidate
	{
		ENYRewardType RewardType = ENYRewardType::StatMaxHp;
		float StatValue = 0.0f;
		TObjectPtr<UNYWeaponDefinition> WeaponDefinition = nullptr;
		ENYRewardWeaponSlot WeaponSlot = ENYRewardWeaponSlot::None;
		float Weight = 1.0f;
		FText DisplayName;
	};

	bool PlayerOwnsWeapon(const UNYWeaponComponent* WeaponComp, const UNYWeaponDefinition* WeaponDef)
	{
		if (!WeaponComp || !WeaponDef)
		{
			return false;
		}

		const FNYWeaponSlot& Primary = WeaponComp->GetPrimarySlot();
		const FNYWeaponSlot& Secondary = WeaponComp->GetSecondarySlot();

		return Primary.Definition == WeaponDef || Secondary.Definition == WeaponDef;
	}

	bool CanOfferNewWeapon(const UNYWeaponComponent* WeaponComp, const UNYWeaponDefinition* WeaponDef)
	{
		if (!WeaponComp || !WeaponDef)
		{
			return false;
		}

		if (WeaponComp->GetSecondarySlot().Definition)
		{
			return false;
		}

		return !PlayerOwnsWeapon(WeaponComp, WeaponDef);
	}

	FText BuildUpgradeDescription(
		const ANYGameModeStage* GameMode,
		const UNYWeaponDefinition* WeaponDef,
		int32 CurrentLevel)
	{
		if (!WeaponDef)
		{
			return FText::GetEmpty();
		}

		const int32 NextLevel = CurrentLevel + 1;
		FNYWeaponLevelRow NextRow;

		if (GameMode && GameMode->TryGetWeaponLevelRow(WeaponDef->WeaponID, NextLevel, NextRow))
		{
			return FText::Format(
				NSLOCTEXT("NYReward", "WeaponUpgradeDesc", "Lv{0} → Lv{1} (DMG x{2})"),
				FText::AsNumber(CurrentLevel),
				FText::AsNumber(NextLevel),
				FText::AsNumber(NextRow.DamageMultiplier));
		}

		return FText::Format(
			NSLOCTEXT("NYReward", "WeaponUpgradeDescSimple", "Lv{0} → Lv{1}"),
			FText::AsNumber(CurrentLevel),
			FText::AsNumber(NextLevel));
	}

	FText ResolveDisplayName(const FNYRewardPoolRow& Row)
	{
		if (!Row.DisplayName.IsEmpty())
		{
			return Row.DisplayName;
		}

		switch (Row.RewardType)
		{
		case ENYRewardType::StatMaxHp:
			return NSLOCTEXT("NYReward", "StatMaxHp", "Max HP Up");
		case ENYRewardType::StatMoveSpeed:
			return NSLOCTEXT("NYReward", "StatMoveSpeed", "Move Speed Up");
		case ENYRewardType::NewWeapon:
			return Row.WeaponDefinition ? Row.WeaponDefinition->DisplayName : NSLOCTEXT("NYReward", "NewWeapon", "New Weapon");
		case ENYRewardType::WeaponUpgrade:
			return NSLOCTEXT("NYReward", "WeaponUpgrade", "Weapon Upgrade");
		default:
			return FText::GetEmpty();
		}
	}

	int32 PickWeightedIndex(const TArray<FCandidate>& Candidates, const TArray<bool>& bUsed)
	{
		float TotalWeight = 0.0f;
		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			if (!bUsed[Index])
			{
				TotalWeight += FMath::Max(0.0f, Candidates[Index].Weight);
			}
		}

		if (TotalWeight <= 0.0f)
		{
			return INDEX_NONE;
		}

		float Roll = FMath::FRandRange(0.0f, TotalWeight);
		float Accumulated = 0.0f;

		for (int32 Index = 0; Index < Candidates.Num(); ++Index)
		{
			if (bUsed[Index])
			{
				continue;
			}

			Accumulated += FMath::Max(0.0f, Candidates[Index].Weight);
			if (Roll <= Accumulated)
			{
				return Index;
			}
		}

		// fallback
		for (int32 Index = Candidates.Num() - 1; Index >= 0; --Index)
		{
			if (!bUsed[Index])
			{
				return Index;
			}
		}

		return INDEX_NONE;
	}
}

TArray<FNYRewardOffer> ANYGameModeStage::GenerateRewardOffers(ANYPlayerStateStage* PlayerState) const
{
	TArray<FNYRewardOffer> Offers;

	if (!PlayerState || RewardOfferCount <= 0)
	{
		return Offers;
	}

	ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(PlayerState->GetPawn());
	UNYWeaponComponent* WeaponComp = Character ? Character->GetWeaponComponent() : nullptr;

	TArray<NYRewardGeneration::FCandidate> Candidates;

	if (RewardPoolDataTable)
	{
		for (const TPair<FName, uint8*>& Pair : RewardPoolDataTable->GetRowMap())
		{
			const FNYRewardPoolRow* Row = reinterpret_cast<const FNYRewardPoolRow*>(Pair.Value);
			if (!Row || Row->MinWave > CurrWave || Row->Weight <= 0.0f)
			{
				continue;
			}

			switch (Row->RewardType)
			{
			case ENYRewardType::StatMaxHp:
			case ENYRewardType::StatMoveSpeed:
			{
				NYRewardGeneration::FCandidate Candidate;
				Candidate.RewardType = Row->RewardType;
				Candidate.StatValue = Row->Value;
				Candidate.Weight = Row->Weight;
				Candidate.DisplayName = NYRewardGeneration::ResolveDisplayName(*Row);
				Candidates.Add(Candidate);
				break;
			}

			case ENYRewardType::NewWeapon:
			{
				if (NYRewardGeneration::CanOfferNewWeapon(WeaponComp, Row->WeaponDefinition))
				{
					NYRewardGeneration::FCandidate Candidate;
					Candidate.RewardType = ENYRewardType::NewWeapon;
					Candidate.WeaponDefinition = Row->WeaponDefinition;
					Candidate.Weight = Row->Weight;
					Candidate.DisplayName = NYRewardGeneration::ResolveDisplayName(*Row);
					Candidates.Add(Candidate);
				}
				break;
			}

			case ENYRewardType::WeaponUpgrade:
			{
				if (WeaponComp)
				{
					if (WeaponComp->CanLevelUpSlot(true))
					{
						const FNYWeaponSlot& Primary = WeaponComp->GetPrimarySlot();
						NYRewardGeneration::FCandidate Candidate;
						Candidate.RewardType = ENYRewardType::WeaponUpgrade;
						Candidate.WeaponDefinition = Primary.Definition;
						Candidate.WeaponSlot = ENYRewardWeaponSlot::Primary;
						Candidate.Weight = Row->Weight;
						Candidate.DisplayName = Primary.Definition
							? Primary.Definition->DisplayName
							: NYRewardGeneration::ResolveDisplayName(*Row);
						Candidates.Add(Candidate);
					}

					if (WeaponComp->CanLevelUpSlot(false))
					{
						const FNYWeaponSlot& Secondary = WeaponComp->GetSecondarySlot();
						NYRewardGeneration::FCandidate Candidate;
						Candidate.RewardType = ENYRewardType::WeaponUpgrade;
						Candidate.WeaponDefinition = Secondary.Definition;
						Candidate.WeaponSlot = ENYRewardWeaponSlot::Secondary;
						Candidate.Weight = Row->Weight;
						Candidate.DisplayName = Secondary.Definition
							? Secondary.Definition->DisplayName
							: NYRewardGeneration::ResolveDisplayName(*Row);
						Candidates.Add(Candidate);
					}
				}
				break;
			}

			default:
				break;
			}
		}
	}

	if (Candidates.Num() <= 0)
	{
		return Offers;
	}

	TArray<bool> bUsed;
	bUsed.Init(false, Candidates.Num());

	const int32 OfferCount = FMath::Min(RewardOfferCount, Candidates.Num());
	Offers.Reserve(OfferCount);

	for (int32 SlotIndex = 0; SlotIndex < OfferCount; ++SlotIndex)
	{
		const int32 PickedIndex = NYRewardGeneration::PickWeightedIndex(Candidates, bUsed);
		if (PickedIndex == INDEX_NONE)
		{
			break;
		}

		bUsed[PickedIndex] = true;
		const NYRewardGeneration::FCandidate& Candidate = Candidates[PickedIndex];

		FNYRewardOffer Offer;
		Offer.SlotIndex = SlotIndex;
		Offer.RewardType = Candidate.RewardType;
		Offer.StatValue = Candidate.StatValue;
		Offer.WeaponDefinition = Candidate.WeaponDefinition;
		Offer.WeaponSlot = Candidate.WeaponSlot;
		Offer.DisplayName = Candidate.DisplayName;

		switch (Candidate.RewardType)
		{
		case ENYRewardType::StatMaxHp:
			Offer.Description = FText::Format(
				NSLOCTEXT("NYReward", "OfferStatMaxHp", "+{0} Max HP"),
				FText::AsNumber(Candidate.StatValue));
			break;

		case ENYRewardType::StatMoveSpeed:
			Offer.Description = FText::Format(
				NSLOCTEXT("NYReward", "OfferStatMoveSpeed", "+{0} Move Speed"),
				FText::AsNumber(Candidate.StatValue));
			break;

		case ENYRewardType::NewWeapon:
			if (Candidate.WeaponDefinition)
			{
				Offer.DisplayName = Candidate.WeaponDefinition->DisplayName;
			}
			Offer.Description = NSLOCTEXT("NYReward", "NewWeaponTag", "New");
			break;

		case ENYRewardType::WeaponUpgrade:
		{
			int32 CurrentLevel = 1;
			if (WeaponComp)
			{
				const FNYWeaponSlot& Slot = Candidate.WeaponSlot == ENYRewardWeaponSlot::Primary
					? WeaponComp->GetPrimarySlot()
					: WeaponComp->GetSecondarySlot();
				CurrentLevel = Slot.Level;
			}

			Offer.Description = NYRewardGeneration::BuildUpgradeDescription(
				this, Candidate.WeaponDefinition, CurrentLevel);
			break;
		}

		default:
			break;
		}

		Offers.Add(Offer);
	}

	return Offers;
}


// Retry
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


// Spawner
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
			SpawnComponent->UpdateSpawnerData(
				MonsterClass,
				WaveData.SpawnInterval,
				WaveData.SpawnCountPerTick,
				WaveData.SpawnRadius);
		}
	}
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
