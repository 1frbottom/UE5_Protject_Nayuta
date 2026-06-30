// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterRanged.h"

#include "Weapons/MonsterWeapons/NYAttackMonsterRanged.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterRanged::ANYMonsterRanged()
{
	LastFireTime = 0.0f;
	RewardRowID = TEXT("Ranged");

	AttackRangeSqrd = FMath::Square(AttackRange);
}

void ANYMonsterRanged::BeginPlay()
{
	Super::BeginPlay();


}

void ANYMonsterRanged::OnRep_ActivationData()
{
	Super::OnRep_ActivationData();

	if (ActivationData.Target != nullptr)
	{
		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(FireCheckTimerHandle, this, &ANYMonsterRanged::CheckAndFire, 0.2f, true);
		}
	}
	else
	{
		GetWorldTimerManager().ClearTimer(FireCheckTimerHandle);
	}
}


// Attack
void ANYMonsterRanged::CheckAndFire()
{
	if (!ActivationData.Target)
		return;

	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < FireRate)
		return;

	float DistSq = FVector::DistSquared(GetActorLocation(), ActivationData.Target->GetActorLocation());
	if (DistSq <= AttackRangeSqrd)
	{
		FireProjectile();
		LastFireTime = CurrentTime;
	}
}

void ANYMonsterRanged::FireProjectile()
{
	if (!ProjectileClass || !ActivationData.Target)
		return;

	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f;
	FRotator SpawnRotation = (ActivationData.Target->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	ANYAttackMonsterRanged* Projectile = GetWorld()->SpawnActor<ANYAttackMonsterRanged>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Projectile)
	{
		Projectile->InitAttackStat(AttackDamage, AttackRange);
	}
}
