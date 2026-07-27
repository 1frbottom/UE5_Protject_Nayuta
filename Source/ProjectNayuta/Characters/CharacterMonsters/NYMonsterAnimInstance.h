// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NYMonsterAnimInstance.generated.h"



class ANYMonsterBase;

/**
 * Shared AnimInstance for pooled monsters.
 * Animation state is derived locally on every machine: monster movement is not replicated,
 * so each client drives the AnimGraph from its own seek simulation instead of network state.
 */
UCLASS()
class PROJECTNAYUTA_API UNYMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;


// Owner
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Owner")
	TObjectPtr<ANYMonsterBase> OwningMonster;


// Locomotion
protected:
	/** Horizontal speed derived from actor location delta; APawn::GetVelocity() is zero without a movement component. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	float GroundSpeed = 0.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Locomotion")
	bool bIsMoving = false;

	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float MovingSpeedThreshold = 3.0f;

	/** Location deltas implying more than this speed are treated as pool teleports, not movement. */
	UPROPERTY(EditDefaultsOnly, Category = "Locomotion")
	float MaxTrackedSpeed = 1000.0f;


// State
protected:
	UPROPERTY(Transient, BlueprintReadOnly, Category = "State")
	bool bIsDead = false;

private:
	FVector PreviousLocation = FVector::ZeroVector;

};
