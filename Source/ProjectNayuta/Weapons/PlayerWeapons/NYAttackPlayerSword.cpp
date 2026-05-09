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
	// 서버에서만 판정하며, 이미 때린 적은 무시
	if (HasAuthority())
		if (OtherActor && OtherActor != GetInstigator())
		{
			UGameplayStatics::ApplyDamage(OtherActor, CurrentDamage, GetInstigatorController(), this, UDamageType::StaticClass());

			// 관통형이면 그대로 두고, 단일 타겟이면 여기서 Destroy() 호출

		}
}