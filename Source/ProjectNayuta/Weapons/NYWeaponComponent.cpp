// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NYWeaponComponent.h"

#include "ProjectNayuta.h"

#include "Engine/OverlapResult.h"
#include "GameFramework/Pawn.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Player/NYPlayerStateStage.h"

#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"



UNYWeaponComponent::UNYWeaponComponent()
{
	SetIsReplicatedByDefault(true);


}

void UNYWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1. Read information from the DataTable
	if (WeaponDataTable && !WeaponID.IsNone())
	{
		static const FString ContextString(TEXT("Weapon Stat Lookup"));
		FWeaponStatRow* RowData = WeaponDataTable->FindRow<FWeaponStatRow>(WeaponID, ContextString);

		if (RowData)
		{
			CurrentAttackClass = RowData->AttackClass;
			CurrentDamage = RowData->BaseDamage;
			CurrentRange = RowData->AttackRange;
			CurrentCooldown = RowData->Cooldown;
		}
	}

	if (GetOwner()->HasAuthority() && CurrentAttackClass)
	{
		GetWorld()->GetTimerManager().SetTimer(AttackTimer, this, &UNYWeaponComponent::FireAttack, CurrentCooldown, true);
	}
}

void UNYWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}

	Super::EndPlay(EndPlayReason);
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

	// true only if block
	bool bHit = GetWorld()->OverlapMultiByChannel(
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
				float Distance = FVector::Dist(StartLoc, Monster->GetActorLocation());
				if (Distance < MinDistance)
				{
					MinDistance = Distance;
					TargetMonster = Monster;
				}
			}
		}

		if (TargetMonster)
		{
			FVector Direction = (TargetMonster->GetActorLocation() - StartLoc).GetSafeNormal();
			FRotator SpawnRotation = Direction.Rotation();
			FVector SpawnLocation = StartLoc + (Direction * 50.0f);

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