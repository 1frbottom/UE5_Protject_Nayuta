// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"

#include "Components/ShapeComponent.h"
#include "ProjectNayuta.h"

void ANYAttackPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	// Find all collision components added in BP and force set them to the player attack channel
	TArray<UShapeComponent*> ShapeComps;
	GetComponents<UShapeComponent>(ShapeComps);

	for (UShapeComponent* Shape : ShapeComps)
	{
		Shape->SetCollisionProfileName(PROFILE_PLAYER_ATTACK);
	}


}
