// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NYAttackBase.h"

#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

ANYAttackBase::ANYAttackBase()
{
	bReplicates = true;

	// Children replace Root with their collision shape, then re-attach this mesh.
	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	SetRootComponent(StaticMeshComp);
	StaticMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ANYAttackBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANYAttackBase, CurrentDamage);
	DOREPLIFETIME(ANYAttackBase, CurrentRange);
}


// Attack
void ANYAttackBase::InitAttackStat(float InDamage, float InRange)
{
	CurrentDamage = InDamage;
	CurrentRange = InRange;
}
