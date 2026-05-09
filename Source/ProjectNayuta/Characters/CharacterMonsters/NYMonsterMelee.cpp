// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterMelee.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"



ANYMonsterMelee::ANYMonsterMelee()
{
	AttackSphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("AttackSphereComp"));
	AttackSphereComp->SetupAttachment(RootComponent);
	AttackSphereComp->InitSphereRadius(60.0f);
	AttackSphereComp->SetCollisionProfileName(PROFILE_MONSTER_ATTACK);


}

void ANYMonsterMelee::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 데미지 판정 타이머 실행
	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ANYMonsterMelee::ProcessMeleeAttack, AttackInterval, true);
	}
}

void ANYMonsterMelee::ProcessMeleeAttack()
{
	TArray<AActor*> OverlappedActors;
	AttackSphereComp->GetOverlappingActors(OverlappedActors);

	for (AActor* Actor : OverlappedActors)
	{
		if (Actor && Actor != this)
		{
			UGameplayStatics::ApplyDamage(Actor, AttackDamage, GetController(), this, UDamageType::StaticClass());
		}
	}
}