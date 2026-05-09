// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterRanged.h"

#include "Weapons/MonsterWeapons/NYAttackMonsterRanged.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterRanged::ANYMonsterRanged()
{
	LastFireTime = 0.0f;

	AttackRangeSqrd = FMath::Square(AttackRange);


}

void ANYMonsterRanged::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(FireCheckTimerHandle, this, &ANYMonsterRanged::CheckAndFire, 0.2f, true);
	}


}

void ANYMonsterRanged::CheckAndFire()
{
	// 1. 타겟 체크
	if (!TargetActor)
		return;

	// 2. 쿨타임 체크
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastFireTime < FireRate)
		return;

	// 3. 거리 체크
	float DistSq = FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation());
	if (DistSq <= AttackRangeSqrd)
	{
		FireProjectile();
		LastFireTime = CurrentTime;
	}
}

void ANYMonsterRanged::FireProjectile()
{
	if (!ProjectileClass || !TargetActor)
		return;

	FVector SpawnLocation = GetActorLocation() + GetActorForwardVector() * 50.0f;
	FRotator SpawnRotation = (TargetActor->GetActorLocation() - SpawnLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;

	ANYAttackMonsterRanged* Projectile = GetWorld()->SpawnActor<ANYAttackMonsterRanged>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Projectile)
	{
		Projectile->InitAttackStat(AttackDamage, AttackRange);
	}
}