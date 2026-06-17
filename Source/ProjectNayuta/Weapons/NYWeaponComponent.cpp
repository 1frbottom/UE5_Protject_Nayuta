// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NYWeaponComponent.h"

#include "ProjectNayuta.h"

#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Game/NYGameStateStage.h"
#include "Player/NYPlayerStateStage.h"

#include "Weapons/NYWeaponDefinition.h"
#include "Weapons/NYWeaponLevelLibrary.h"
#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"

UNYWeaponComponent::UNYWeaponComponent()
{
	SetIsReplicatedByDefault(true);
}

void UNYWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UNYWeaponComponent, PrimarySlot);
	DOREPLIFETIME(UNYWeaponComponent, SecondarySlot);
}

void UNYWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	ApplyWeaponDefinition();
	RefreshAttackTimer();
}

void UNYWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
}

void UNYWeaponComponent::SetWeaponDefinition(UNYWeaponDefinition* NewDefinition)
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	PrimarySlot.Definition = NewDefinition;
	PrimarySlot.Level = 1;
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();
}

void UNYWeaponComponent::SetSecondaryWeaponDefinition(UNYWeaponDefinition* NewDefinition)
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	SecondarySlot.Definition = NewDefinition;
	SecondarySlot.Level = 1;
	NotifyWeaponSlotsChanged();
}

void UNYWeaponComponent::SwapWeaponSlots()
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	Swap(PrimarySlot, SecondarySlot);
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();
}

int32 UNYWeaponComponent::GetMaxWeaponLevelForSlot(const FNYWeaponSlot& Slot) const
{
	if (!Slot.Definition || Slot.Definition->WeaponID.IsNone())
	{
		return 1;
	}

	const UWorld* World = GetWorld();
	const ANYGameStateStage* GS = World ? World->GetGameState<ANYGameStateStage>() : nullptr;

	return NYWeaponLevel::GetMaxLevel(GS ? GS->WeaponLevelDataTable : nullptr, Slot.Definition->WeaponID);
}

int32 UNYWeaponComponent::GetMaxWeaponLevel() const
{
	return GetMaxWeaponLevelForSlot(PrimarySlot);
}

bool UNYWeaponComponent::CanLevelUpWeapon() const
{
	return PrimarySlot.Definition && PrimarySlot.Level < GetMaxWeaponLevel();
}

bool UNYWeaponComponent::LevelUpWeapon()
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority() || !CanLevelUpWeapon())
	{
		return false;
	}

	PrimarySlot.Level++;
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();

	return true;
}

void UNYWeaponComponent::ResetWeaponLevel()
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	const bool bPrimaryNeedsReset = PrimarySlot.Level != 1;
	const bool bSecondaryNeedsReset = SecondarySlot.Definition != nullptr || SecondarySlot.Level != 1;

	if (!bPrimaryNeedsReset && !bSecondaryNeedsReset)
	{
		return;
	}

	PrimarySlot.Level = 1;
	SecondarySlot.Definition = nullptr;
	SecondarySlot.Level = 1;
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();
}

void UNYWeaponComponent::OnRep_WeaponSlots()
{
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();
}

void UNYWeaponComponent::NotifyWeaponLevelChanged()
{
	OnWeaponLevelChanged.Broadcast(PrimarySlot.Level);
}

void UNYWeaponComponent::NotifyWeaponSlotsChanged()
{
	OnWeaponSlotsChanged.Broadcast();
}

void UNYWeaponComponent::ApplyWeaponDefinition()
{
	CurrentAttackClass = nullptr;
	CurrentDamage = 0.0f;
	CurrentRange = 0.0f;
	CurrentCooldown = 0.0f;

	if (!PrimarySlot.Definition)
	{
		return;
	}

	FNYWeaponLevelRow LevelRow;
	const UWorld* World = GetWorld();
	const ANYGameStateStage* GS = World ? World->GetGameState<ANYGameStateStage>() : nullptr;
	const UDataTable* WeaponLevelTable = GS ? GS->WeaponLevelDataTable : nullptr;

	if (!NYWeaponLevel::TryGetRow(
		WeaponLevelTable, PrimarySlot.Definition->WeaponID, PrimarySlot.Level, LevelRow))
	{
		LevelRow.DamageMultiplier = 1.0f;
		LevelRow.RangeMultiplier = 1.0f;
		LevelRow.CooldownMultiplier = 1.0f;
	}

	CurrentAttackClass = PrimarySlot.Definition->AttackClass;
	CurrentDamage = PrimarySlot.Definition->BaseDamage * LevelRow.DamageMultiplier;
	CurrentRange = PrimarySlot.Definition->AttackRange * LevelRow.RangeMultiplier;
	CurrentCooldown = FMath::Max(
		0.01f, PrimarySlot.Definition->Cooldown * LevelRow.CooldownMultiplier);
}

void UNYWeaponComponent::RefreshAttackTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(AttackTimer);
	}

	if (!GetOwner() || !GetOwner()->HasAuthority() || !CurrentAttackClass || CurrentCooldown <= 0.0f)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		AttackTimer, this, &UNYWeaponComponent::FireAttack, CurrentCooldown, true);
}

void UNYWeaponComponent::FireAttack()
{
	if (!CurrentAttackClass || !GetOwner()->HasAuthority())
	{
		return;
	}

	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return;
	}

	// Server: only fire while the owning player is in the Alive phase.
	if (ANYPlayerStateStage* PS = OwnerPawn->GetPlayerState<ANYPlayerStateStage>())
	{
		if (PS->GetPlayerPhase() != ENYPlayerPhase::Alive)
		{
			return;
		}
	}

	FVector StartLoc = GetOwner()->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	GetWorld()->OverlapMultiByChannel(
		OverlapResults, StartLoc, FQuat::Identity, ECC_PLAYERATTACK,
		FCollisionShape::MakeSphere(CurrentRange), CollisionParams);

	if (OverlapResults.Num() > 0)
	{
		ANYMonsterBase* TargetMonster = nullptr;
		float MinDistance = CurrentRange + 1.0f;

		for (const FOverlapResult& Result : OverlapResults)
		{
			ANYMonsterBase* Monster = Cast<ANYMonsterBase>(Result.GetActor());
			if (Monster)
			{
				const float Distance = FVector::Dist(StartLoc, Monster->GetActorLocation());
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					TargetMonster = Monster;
				}
			}
		}

		if (TargetMonster)
		{
			const FVector Direction = (TargetMonster->GetActorLocation() - StartLoc).GetSafeNormal();
			const FRotator SpawnRotation = Direction.Rotation();
			const FVector SpawnLocation = StartLoc + (Direction * 50.0f);
			const FTransform SpawnTransform(SpawnRotation, SpawnLocation);

			ANYAttackPlayerBase* SpawnedAttack = GetWorld()->SpawnActorDeferred<ANYAttackPlayerBase>(
				CurrentAttackClass,
				SpawnTransform,
				GetOwner(),
				OwnerPawn,
				ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

			if (SpawnedAttack)
			{
				SpawnedAttack->InitAttackStat(CurrentDamage, CurrentRange);
				SpawnedAttack->FinishSpawning(SpawnTransform);
			}
		}
	}
}

