// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NYWeaponComponent.h"

#include "ProjectNayuta.h"

#include "Animation/AnimMontage.h"
#include "GameFramework/Pawn.h"
#include "Net/UnrealNetwork.h"

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

	StartFireTimerIfNeeded(false);
}

void UNYWeaponComponent::SetWantsToFire(bool bNewWantsToFire)
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	if (bWantsToFire == bNewWantsToFire)
	{
		return;
	}

	bWantsToFire = bNewWantsToFire;

	if (!bWantsToFire)
	{
		if (UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(AttackTimer);
		}
		return;
	}

	StartFireTimerIfNeeded(true);
}

void UNYWeaponComponent::SetAimDirection(const FVector& NewAimDir)
{
	// Server
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	FVector Flattened = NewAimDir;
	Flattened.Z = 0.0f;
	if (Flattened.Normalize())
	{
		AimDirection = Flattened;
	}
}

FVector UNYWeaponComponent::GetAimDirection() const
{
	if (AimDirection.SizeSquared2D() > KINDA_SMALL_NUMBER)
	{
		return AimDirection;
	}

	if (const APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		const FVector Forward = OwnerPawn->GetActorForwardVector().GetSafeNormal2D();
		if (!Forward.IsNearlyZero())
		{
			return Forward;
		}
	}

	return FVector::ForwardVector;
}

void UNYWeaponComponent::StartFireTimerIfNeeded(bool bFireImmediately)
{
	// Server
	UWorld* World = GetWorld();
	if (!World || !GetOwner() || !GetOwner()->HasAuthority() || !bWantsToFire || !CurrentAttackClass || CurrentCooldown <= 0.0f)
	{
		return;
	}

	// Tapping must not bypass the cooldown: hold back the first shot by whatever is left of it.
	float FirstDelay = CurrentCooldown;
	if (bFireImmediately)
	{
		const float Elapsed = (LastFireServerTime < 0.0f)
			? CurrentCooldown
			: (World->GetTimeSeconds() - LastFireServerTime);
		FirstDelay = FMath::Clamp(CurrentCooldown - Elapsed, 0.0f, CurrentCooldown);
	}

	if (FirstDelay <= 0.0f)
	{
		FireAttack();
		FirstDelay = CurrentCooldown;
	}

	World->GetTimerManager().SetTimer(
		AttackTimer, this, &UNYWeaponComponent::FireAttack, CurrentCooldown, true, FirstDelay);
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
	UWorld* World = GetWorld();
	if (!World || !CanFireAttack())
	{
		return;
	}

	if (World->GetTimerManager().IsTimerActive(AttackCommitTimerHandle))
	{
		return;
	}

	LastFireServerTime = World->GetTimeSeconds();

	UAnimMontage* MontageToPlay = PrimarySlot.Definition ? PrimarySlot.Definition->AttackMontage : nullptr;
	if (ANYCharacterPlayer* OwnerCharacter = Cast<ANYCharacterPlayer>(GetOwner()))
	{
		OwnerCharacter->PlayAttackMontage(MontageToPlay);
	}

	const float CommitDelay = GetAttackCommitDelay(MontageToPlay, AttackCommitNotifyName);
	if (CommitDelay > 0.0f)
	{
		World->GetTimerManager().SetTimer(
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

	SpawnAttackToward(GetAimDirection());
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

void UNYWeaponComponent::SpawnAttackToward(const FVector& Direction)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (!OwnerPawn || !CurrentAttackClass)
	{
		return;
	}

	FVector FlatDirection = Direction;
	FlatDirection.Z = 0.0f;
	if (!FlatDirection.Normalize())
	{
		FlatDirection = GetAimDirection();
	}

	const FVector StartLoc = GetOwner()->GetActorLocation();
	const FRotator SpawnRotation = FlatDirection.Rotation();
	const FVector SpawnLocation = StartLoc + (FlatDirection * 50.0f);
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
