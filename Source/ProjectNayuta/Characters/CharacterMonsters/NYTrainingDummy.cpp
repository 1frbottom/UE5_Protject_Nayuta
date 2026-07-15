// Fill out your copyright notice in the Description page of Project Settings.

#include "Characters/CharacterMonsters/NYTrainingDummy.h"

#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ANYTrainingDummy::ANYTrainingDummy()
{
	MoveSpeed = 0.0f;
	MaxHp = 1000.0f;
	PrimaryActorTick.bCanEverTick = false;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyMesh(
		TEXT("/Game/Characters/Monster/SM_Monster"));
	if (BodyMesh.Succeeded() && SphereComp)
	{
		SphereComp->SetStaticMesh(BodyMesh.Object);
		SphereComp->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f));
		SphereComp->SetRelativeScale3D(FVector(1.0f));
	}
}

void ANYTrainingDummy::BeginPlay()
{
	Super::BeginPlay();
	ForceActivePresentation();
}

void ANYTrainingDummy::OnRep_ActivationData()
{
	// Ignore pool hide/show — training dummies are level-placed, not pooled.
	ForceActivePresentation();
}

float ANYTrainingDummy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	CurrentHp -= DamageAmount;
	if (CurrentHp <= 0.0f)
	{
		CurrentHp = MaxHp;
	}

	OnRep_CurrentHp();
	return DamageAmount;
}

void ANYTrainingDummy::ForceActivePresentation()
{
	SetNetDormancy(DORM_Awake);
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
	SetActorTickEnabled(false);
	OnRep_CurrentHp();
}
