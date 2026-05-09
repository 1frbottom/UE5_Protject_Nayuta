// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NYWeaponComponent.h"

#include "ProjectNayuta.h"

#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Engine/OverlapResult.h"



UNYWeaponComponent::UNYWeaponComponent()
{
	SetIsReplicatedByDefault(true);


}

void UNYWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// 1. 데이터 테이블에서 정보 불러오기
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

void UNYWeaponComponent::FireAttack()
{
	if (!CurrentAttackClass)
		return;

	FVector StartLoc = GetOwner()->GetActorLocation();
	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(GetOwner());

	bool bHit = GetWorld()->OverlapMultiByChannel(
		OverlapResults, StartLoc, FQuat::Identity, ECC_MONSTER,
		FCollisionShape::MakeSphere(CurrentRange), CollisionParams);

	if (bHit)
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

			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = Cast<APawn>(GetOwner());

			ANYAttackPlayerBase* SpawnedAttack = GetWorld()->SpawnActor<ANYAttackPlayerBase>(CurrentAttackClass, SpawnLocation, SpawnRotation, SpawnParams);

			// 스폰 직후 스탯 주입
			if (SpawnedAttack)
			{
				SpawnedAttack->InitAttackStat(CurrentDamage, CurrentRange);
			}
		}
	}
}