// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MonsterWeapons/NYAttackMonsterBase.h"

#include "Components/ShapeComponent.h"
#include "ProjectNayuta.h"

void ANYAttackMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	// Find all collision components added in BP and force set them to the monster attack channel
	TArray<UShapeComponent*> ShapeComps;
	GetComponents<UShapeComponent>(ShapeComps);

	for (UShapeComponent* Shape : ShapeComps)
	{
		Shape->SetCollisionProfileName(PROFILE_MONSTER_ATTACK);
	}


}