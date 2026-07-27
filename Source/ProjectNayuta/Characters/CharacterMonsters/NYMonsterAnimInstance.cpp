// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterAnimInstance.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"



void UNYMonsterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwningMonster = Cast<ANYMonsterBase>(TryGetPawnOwner());

	if (OwningMonster)
	{
		PreviousLocation = OwningMonster->GetActorLocation();
	}
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

		PreviousLocation = OwningMonster->GetActorLocation();
	}

	const FVector CurrentLocation = OwningMonster->GetActorLocation();

	if (DeltaSeconds > 0.0f)
	{
		const float FrameDistance = FVector::Dist2D(CurrentLocation, PreviousLocation);
		const float MaxFrameDistance = MaxTrackedSpeed * DeltaSeconds;

		// 1. Speed
		// Pool activation snaps the actor to a spawn point; that jump is not locomotion.
		GroundSpeed = (FrameDistance > MaxFrameDistance) ? 0.0f : FrameDistance / DeltaSeconds;
	}

	PreviousLocation = CurrentLocation;

	// 2. Moving
	bIsMoving = GroundSpeed > MovingSpeedThreshold;

	// 3. Dead
	bIsDead = OwningMonster->GetCurrentHp() <= 0.0f;
}
