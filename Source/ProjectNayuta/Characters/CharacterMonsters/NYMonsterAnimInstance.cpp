// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterAnimInstance.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"



void UNYMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningMonster = Cast<ANYMonsterBase>(TryGetPawnOwner());
}

void UNYMonsterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwningMonster == nullptr)
	{
		OwningMonster = Cast<ANYMonsterBase>(TryGetPawnOwner());

		if (OwningMonster == nullptr)
		{
			return;
		}
	}

	// 1. Speed
	// UNYMonsterMovementComponent publishes only swept distance, so pool activation snaps
	// (which move the actor directly) never register as locomotion.
	GroundSpeed = OwningMonster->GetVelocity().Size2D();

	// 2. Moving
	bIsMoving = GroundSpeed > MovingSpeedThreshold;

	// 3. Dead
	bIsDead = OwningMonster->GetCurrentHp() <= 0.0f;
}
