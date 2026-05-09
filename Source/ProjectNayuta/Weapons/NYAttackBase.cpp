// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/NYAttackBase.h"

#include "Kismet/GameplayStatics.h"



ANYAttackBase::ANYAttackBase()
{
	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultSceneRoot"));


}

void ANYAttackBase::InitAttackStat(float InDamage, float InRange)
{
	CurrentDamage = InDamage;
	CurrentRange = InRange;
}