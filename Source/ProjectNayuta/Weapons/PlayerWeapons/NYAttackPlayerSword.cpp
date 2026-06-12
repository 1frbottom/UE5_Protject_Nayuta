// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/PlayerWeapons/NYAttackPlayerSword.h"

#include "ProjectNayuta.h"

#include "Components/SphereComponent.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"



ANYAttackPlayerSword::ANYAttackPlayerSword()
{
	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	RootComponent = SphereComp;
	SphereComp->InitSphereRadius(100.f);
}

void ANYAttackPlayerSword::BeginPlay()
{
	Super::BeginPlay();

	if (APawn* InstigatorPawn = GetInstigator())
	{
		SetActorLocation(InstigatorPawn->GetActorLocation());
	}

	SphereComp->SetSphereRadius(CurrentRange);
	SetLifeSpan(MeleeVisualDuration);

	// Server
	if (HasAuthority())
	{
		ApplyMeleeDamageInRange();
	}
}

void ANYAttackPlayerSword::ApplyMeleeDamageInRange()
{
	UWorld* World = GetWorld();
	if (!World || CurrentRange <= 0.0f)
	{
		return;
	}

	const FVector Origin = GetActorLocation();
	const float RangeSqrd = FMath::Square(CurrentRange);
	const bool bFullCircle = MeleeSweepAngle >= 360.0f;

	FVector Forward = GetActorForwardVector();
	Forward.Z = 0.0f;
	if (!Forward.Normalize())
	{
		if (APawn* InstigatorPawn = GetInstigator())
		{
			Forward = InstigatorPawn->GetActorForwardVector();
			Forward.Z = 0.0f;
			Forward.Normalize();
		}
	}

	const float CosThreshold = FMath::Cos(FMath::DegreesToRadians(MeleeSweepAngle * 0.5f));

	TArray<FOverlapResult> OverlapResults;
	FCollisionQueryParams CollisionParams;
	CollisionParams.AddIgnoredActor(this);
	if (AActor* InstigatorActor = GetInstigator())
	{
		CollisionParams.AddIgnoredActor(InstigatorActor);
	}

	World->OverlapMultiByChannel(
		OverlapResults,
		Origin,
		FQuat::Identity,
		ECC_PLAYERATTACK,
		FCollisionShape::MakeSphere(CurrentRange),
		CollisionParams);

	for (const FOverlapResult& Result : OverlapResults)
	{
		AActor* OtherActor = Result.GetActor();
		if (!OtherActor || OtherActor == GetInstigator())
		{
			continue;
		}

		if (FVector::DistSquared2D(Origin, OtherActor->GetActorLocation()) > RangeSqrd)
		{
			continue;
		}

		if (!bFullCircle)
		{
			FVector ToTarget = OtherActor->GetActorLocation() - Origin;
			ToTarget.Z = 0.0f;
			if (!ToTarget.Normalize())	// prevent div0
			{
				continue;
			}

			if (FVector::DotProduct(Forward, ToTarget) < CosThreshold)	// not in range of sector
			{
				continue;
			}
		}

		UGameplayStatics::ApplyDamage(OtherActor, CurrentDamage, GetInstigatorController(), this, UDamageType::StaticClass());
	}
}
