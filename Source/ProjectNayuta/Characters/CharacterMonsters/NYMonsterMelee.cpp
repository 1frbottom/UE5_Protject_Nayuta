// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterMelee.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterMelee::ANYMonsterMelee()
{


}

void ANYMonsterMelee::BeginPlay()
{
	Super::BeginPlay();

}

void ANYMonsterMelee::ProcessMeleeAttack()
{
	if (TargetActor == nullptr)
		return;

	// 단순 거리벡터로 변경
	float DistSq = FVector::DistSquared(GetActorLocation(), TargetActor->GetActorLocation());

	float AttackRangeSq = FMath::Square(100.0f);

	if (DistSq <= AttackRangeSq)
	{
		UGameplayStatics::ApplyDamage(TargetActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
	}

}
