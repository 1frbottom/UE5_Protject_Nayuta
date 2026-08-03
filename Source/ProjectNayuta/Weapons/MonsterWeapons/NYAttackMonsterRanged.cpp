// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MonsterWeapons/NYAttackMonsterRanged.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"



ANYAttackMonsterRanged::ANYAttackMonsterRanged()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->InitSphereRadius(15.0f);
	SphereComp->SetCollisionProfileName(PROFILE_MONSTER_ATTACK);

	StaticMeshComp->SetupAttachment(RootComponent);

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 800.0f;
	// Unlimited: SetLaunchVelocity supplies the actual speed, and a lobbed arc can exceed this otherwise.
	ProjectileMovementComp->MaxSpeed = 0.0f;
	ProjectileMovementComp->ProjectileGravityScale = 1.0f;

	InitialLifeSpan = 5.0f;
}

void ANYAttackMonsterRanged::SetLaunchVelocity(const FVector& InVelocity)
{
	ProjectileMovementComp->Velocity = InVelocity;
}

void ANYAttackMonsterRanged::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		SphereComp->OnComponentBeginOverlap.AddDynamic(this, &ANYAttackMonsterRanged::OnOverlapBegin);
	}


}


// Attack
void ANYAttackMonsterRanged::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (HasAuthority())
		if(OtherActor && OtherActor != GetInstigator())
		{
			UGameplayStatics::ApplyDamage(OtherActor, CurrentDamage, GetInstigatorController(), this, UDamageType::StaticClass());

			Destroy();
		}
}