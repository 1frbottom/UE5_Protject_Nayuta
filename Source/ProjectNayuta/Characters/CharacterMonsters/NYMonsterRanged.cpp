// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterRanged.h"

#include "Weapons/MonsterWeapons/NYAttackMonsterRanged.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterRanged::ANYMonsterRanged()
{
	RewardRowID = TEXT("Ranged");

	AttackDamage = 15.0f;
	AttackRange = 500.0f;
	AttackInterval = 2.0f;
}

void ANYMonsterRanged::BeginPlay()
{
	Super::BeginPlay();


}

// Attack
void ANYMonsterRanged::PerformAttack()
{
	// Server-only: base's ProcessAttack only calls this after CanAttack() (target, stagger, range).
	FireProjectile();
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

		FVector LaunchVelocity;
		if (UGameplayStatics::SuggestProjectileVelocity_CustomArc(
			this, LaunchVelocity, SpawnLocation, ActivationData.Target->GetActorLocation(), 0.0f, ProjectileArcParam))
		{
			Projectile->SetLaunchVelocity(LaunchVelocity);
		}
	}
}
