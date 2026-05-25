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

void ANYMonsterMelee::ResetState()
{
	Super::ResetState();

	if (HasAuthority())
	{
		GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ANYMonsterMelee::ProcessMeleeAttack, AttackInterval, true);
	}
}

void ANYMonsterMelee::Deactivate()
{
	Super::Deactivate();

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
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