// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterMelee.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterMelee::ANYMonsterMelee()
{
	RewardRowID = TEXT("Melee");
}

void ANYMonsterMelee::BeginPlay()
{
	Super::BeginPlay();

}

void ANYMonsterMelee::OnRep_ActivationData()
{
    Super::OnRep_ActivationData();

    if (ActivationData.Target != nullptr)
    {
        if (HasAuthority())
        {
            GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ANYMonsterMelee::ProcessMeleeAttack, AttackInterval, true);
        }
    }
    else
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
    }
}


// Attack
void ANYMonsterMelee::StopAttackOnServer()
{
	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
}

void ANYMonsterMelee::ProcessMeleeAttack()
{
	if (ActivationData.Target == nullptr)
		return;

	float DistSq = FVector::DistSquared(GetActorLocation(), ActivationData.Target->GetActorLocation());

	float AttackRangeSq = FMath::Square(100.0f);

	if (DistSq <= AttackRangeSq)
	{
		UGameplayStatics::ApplyDamage(ActivationData.Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
	}

}
