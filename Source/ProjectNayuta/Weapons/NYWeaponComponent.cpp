// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NYWeaponComponent.h"

#include "ProjectNayuta.h"

#include "Animation/AnimMontage.h"
#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
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


// Weapon
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

bool UNYWeaponComponent::CanSwapWeaponSlots() const
{
	return SecondarySlot.Definition != nullptr;
}

void UNYWeaponComponent::SwapWeaponSlots()
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority() || !CanSwapWeaponSlots())
	{
		return;
	}

	Swap(PrimarySlot, SecondarySlot);
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();
}

int32 UNYWeaponComponent::GetMaxWeaponLevel() const
{
	return GetMaxWeaponLevelForSlot(PrimarySlot);
}

bool UNYWeaponComponent::CanLevelUpSlot(bool bPrimary) const
{
	const FNYWeaponSlot& Slot = GetSlot(bPrimary);
	
	return Slot.Definition && Slot.Level < GetMaxWeaponLevelForSlot(Slot);
}

bool UNYWeaponComponent::CanLevelUpWeapon() const
{
	return CanLevelUpSlot(true);
}

bool UNYWeaponComponent::LevelUpSlot(bool bPrimary)
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority() || !CanLevelUpSlot(bPrimary))
	{
		return false;
	}

	FNYWeaponSlot& Slot = bPrimary ? PrimarySlot : SecondarySlot;
	Slot.Level++;

	if (bPrimary)
	{
		ApplyWeaponDefinition();
		RefreshAttackTimer();
	}

	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();

	return true;
}

bool UNYWeaponComponent::LevelUpWeapon()
{
	return LevelUpSlot(true);
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
		World->GetTimerManager().ClearTimer(AttackCommitTimerHandle);
	}

	if (!GetOwner() || !GetOwner()->HasAuthority() || !CurrentAttackClass || CurrentCooldown <= 0.0f)
	{
		return;
	}

	GetWorld()->GetTimerManager().SetTimer(
		AttackTimer, this, &UNYWeaponComponent::FireAttack, CurrentCooldown, true);
}

void UNYWeaponComponent::NotifyWeaponLevelChanged()
{
	OnWeaponLevelChanged.Broadcast(PrimarySlot.Level);
}

void UNYWeaponComponent::NotifyWeaponSlotsChanged()
{
	OnWeaponSlotsChanged.Broadcast();
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

const FNYWeaponSlot& UNYWeaponComponent::GetSlot(bool bPrimary) const
{
	return bPrimary ? PrimarySlot : SecondarySlot;
}

void UNYWeaponComponent::OnRep_WeaponSlots()
{
	ApplyWeaponDefinition();
	RefreshAttackTimer();
	NotifyWeaponLevelChanged();
	NotifyWeaponSlotsChanged();
}

void UNYWeaponComponent::FireAttack()
{
	// Server
	if (!CanFireAttack())
	{
		return;
	}

	if (!FindNearestTargetInRange())
	{
		return;
	}

	UAnimMontage* MontageToPlay = PrimarySlot.Definition ? PrimarySlot.Definition->AttackMontage : nullptr;
	if (ANYCharacterPlayer* OwnerCharacter = Cast<ANYCharacterPlayer>(GetOwner()))
	{
		OwnerCharacter->PlayAttackMontage(MontageToPlay);
	}

	const float CommitDelay = GetAttackCommitDelay(MontageToPlay, AttackCommitNotifyName);
	if (CommitDelay > 0.0f)
	{
		GetWorld()->GetTimerManager().SetTimer(
			AttackCommitTimerHandle,
			this,
			&UNYWeaponComponent::CommitAttackOnServer,
			CommitDelay,
			false);
	}
	else
	{
		CommitAttackOnServer();
	}
}

void UNYWeaponComponent::CommitAttackOnServer()
{
	// Server: windup finished. Skip if the owner can no longer attack.
	if (!CanFireAttack())
	{
		return;
	}

	ANYMonsterBase* TargetMonster = FindNearestTargetInRange();
	if (!TargetMonster)
	{
		return;
	}

	SpawnAttackToward(TargetMonster);
}

bool UNYWeaponComponent::CanFireAttack() const
{
	if (!CurrentAttackClass || !GetOwner() || !GetOwner()->HasAuthority())
	{
		return false;
	}

	const APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn)
	{
		return false;
	}

	if (const ANYPlayerStateStage* PS = OwnerPawn->GetPlayerState<ANYPlayerStateStage>())
	{
		return PS->CanControlPawn();
	}

	return true;
}

ANYMonsterBase* UNYWeaponComponent::FindNearestTargetInRange() const
{
	AActor* OwnerActor = GetOwner();
	UWorld* World = GetWorld();
	if (!OwnerActor || !World || CurrentRange <= 0.0f)
	{
		return nullptr;
	}

	const FVector StartLoc = OwnerActor->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(OwnerActor);

	World->OverlapMultiByChannel(
		OverlapResults, StartLoc, FQuat::Identity, ECC_PLAYERATTACK,
		FCollisionShape::MakeSphere(CurrentRange), CollisionParams);

	ANYMonsterBase* TargetMonster = nullptr;
	float MinDistance = CurrentRange + 1.0f;

	for (const FOverlapResult& Result : OverlapResults)
	{
		ANYMonsterBase* Monster = Cast<ANYMonsterBase>(Result.GetActor());
		if (!Monster)
		{
			continue;
		}

		const float Distance = FVector::Dist(StartLoc, Monster->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			TargetMonster = Monster;
		}
	}

	return TargetMonster;
}

void UNYWeaponComponent::SpawnAttackToward(ANYMonsterBase* TargetMonster)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!TargetMonster || !OwnerPawn || !CurrentAttackClass)
	{
		return;
	}

	const FVector StartLoc = GetOwner()->GetActorLocation();
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

float UNYWeaponComponent::GetAttackCommitDelay(const UAnimMontage* Montage, FName NotifyName)
{
	if (!Montage || NotifyName.IsNone())
	{
		return 0.0f;
	}

	for (const FAnimNotifyEvent& Event : Montage->Notifies)
	{
		if (Event.NotifyName == NotifyName)
		{
			return FMath::Max(Event.GetTriggerTime(), 0.0f);
		}
	}

	for (const FSlotAnimationTrack& SlotTrack : Montage->SlotAnimTracks)
	{
		for (const FAnimSegment& Segment : SlotTrack.AnimTrack.AnimSegments)
		{
			const UAnimSequenceBase* Sequence = Segment.GetAnimReference();
			if (!Sequence)
			{
				continue;
			}

			for (const FAnimNotifyEvent& Event : Sequence->Notifies)
			{
				if (Event.NotifyName != NotifyName)
				{
					continue;
				}

				const float MontageTime = Segment.StartPos
					+ (Event.GetTriggerTime() - Segment.AnimStartTime) / Segment.GetValidPlayRate();
				return FMath::Max(MontageTime, 0.0f);
			}
		}
	}

	return 0.0f;
}
