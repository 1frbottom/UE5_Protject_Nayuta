// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterMelee.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterMelee::ANYMonsterMelee()
{
	RewardRowID = TEXT("Melee");

	AttackDamage = 30.0f;
	AttackRange = 200.0f;
	AttackInterval = 0.5f;
}

void ANYMonsterMelee::BeginPlay()
{
	Super::BeginPlay();

}


// Attack
void ANYMonsterMelee::PerformAttack()
{
	// Server-only: base's ProcessAttack only calls this after CanAttack() (target, stagger, range).
	UGameplayStatics::ApplyDamage(ActivationData.Target, AttackDamage, GetController(), this, UDamageType::StaticClass());
}
