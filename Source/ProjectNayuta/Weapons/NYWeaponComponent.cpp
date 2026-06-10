// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NYWeaponComponent.h"

#include "ProjectNayuta.h"

#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Player/NYPlayerStateStage.h"

#include "Weapons/NYWeaponDefinition.h"
#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"

UNYWeaponComponent::UNYWeaponComponent()
{
	SetIsReplicatedByDefault(true);
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
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	WeaponDefinition = NewDefinition;
	ApplyWeaponDefinition();
	RefreshAttackTimer();
}

void UNYWeaponComponent::ApplyWeaponDefinition()
{
	CurrentAttackClass = nullptr;
	CurrentDamage = 0.0f;
	CurrentRange = 0.0f;
	CurrentCooldown = 0.0f;

	if (!WeaponDefinition)
	{
		return;
	}

	CurrentAttackClass = WeaponDefinition->AttackClass;
	CurrentDamage = WeaponDefinition->BaseDamage;
	CurrentRange = WeaponDefinition->AttackRange;
	CurrentCooldown = FMath::Max(0.01f, WeaponDefinition->Cooldown);
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
