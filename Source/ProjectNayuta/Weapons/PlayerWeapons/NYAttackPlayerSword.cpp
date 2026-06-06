// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/PlayerWeapons/NYAttackPlayerSword.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"



ANYAttackPlayerSword::ANYAttackPlayerSword()
{
	BoxComp = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxComp"));
	RootComponent = BoxComp;
	BoxComp->SetBoxExtent(FVector(20.f, 50.f, 20.f));

	ProjectileMovementComp = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComp"));
	ProjectileMovementComp->InitialSpeed = 1000.f;
	ProjectileMovementComp->MaxSpeed = 1000.f;
	ProjectileMovementComp->ProjectileGravityScale = 0.0f;

	InitialLifeSpan = 1.0f;

}

void ANYAttackPlayerSword::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ANYAttackPlayerSword::OnOverlapBegin);
	}
}

void ANYAttackPlayerSword::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Only check on the server, ignore already hit enemies
	if (HasAuthority())
		if (OtherActor && OtherActor != GetInstigator())
		{
			UGameplayStatics::ApplyDamage(OtherActor, CurrentDamage, GetInstigatorController(), this, UDamageType::StaticClass());

			// If it's a penetrating type, leave it as is, if it's a single target, call Destroy() here

		}

}