// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MonsterWeapons/NYAttackMonsterRanged.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"



ANYAttackMonsterRanged::ANYAttackMonsterRanged()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->InitSphereRadius(15.0f);
	SphereComp->SetCollisionProfileName(PROFILE_MONSTER_ATTACK);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 800.0f;
	ProjectileMovementComp->MaxSpeed = 800.0f;
	ProjectileMovementComp->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 5.0f; // 5초 뒤 자동 소멸


}

void ANYAttackMonsterRanged::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ANYAttackMonsterRanged::OnOverlapBegin);
	}


}

void ANYAttackMonsterRanged::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
		if(OtherActor && OtherActor != GetInstigator())
		{
			UGameplayStatics::ApplyDamage(OtherActor, CurrentDamage, GetInstigatorController(), this, UDamageType::StaticClass());

			Destroy();
		}
}