// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/PlayerWeapons/NYAttackPlayerAxe.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

ANYAttackPlayerAxe::ANYAttackPlayerAxe()
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

void ANYAttackPlayerAxe::BeginPlay()
{
	Super::BeginPlay();

	// Server
	if (HasAuthority())
	{
		BoxComp->OnComponentBeginOverlap.AddDynamic(this, &ANYAttackPlayerAxe::OnOverlapBegin);
	}
}


// Attack
void ANYAttackPlayerAxe::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// Server
	if (HasAuthority())
	{
		if (OtherActor && OtherActor != GetInstigator())
		{
			UGameplayStatics::ApplyDamage(OtherActor, CurrentDamage, GetInstigatorController(), this, UDamageType::StaticClass());
		}
	}
}
