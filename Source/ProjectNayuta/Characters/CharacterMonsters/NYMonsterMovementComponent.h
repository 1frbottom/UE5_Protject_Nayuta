// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "NYMonsterMovementComponent.generated.h"



class UCapsuleComponent;

/**
 * Ground movement for pooled horde monsters, sized for hundreds of concurrent instances.
 *
 * UCharacterMovementComponent is far too heavy to run at that count, and its network prediction
 * would be wasted: monster movement is never replicated, so every machine simulates the same
 * seeded path locally. What is left is exactly what CMC would have provided and this has to
 * supply by hand — floor probing, gravity, stair step-up and slope sliding.
 *
 * The owner drives every update from its own Tick; this component never ticks itself.
 */
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTNAYUTA_API UNYMonsterMovementComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	UNYMonsterMovementComponent();

	virtual void SetUpdatedComponent(USceneComponent* NewUpdatedComponent) override;


// Movement
public:
	/** Spawn/activate helper: snaps Z to the floor when one is found, otherwise returns Location. */
	FVector SnapLocationToFloor(const FVector& Location) const;

	/**
	 * Sweeps Delta; on a blocking hit against a walkable slope (see WalkableNormalZ), slides the
	 * remainder along the surface instead of stopping dead, so real walls still block movement.
	 * A near-vertical hit is climbed only by the measured step height, up to MaxFloorSnapUp.
	 */
	void MoveHorizontal(const FVector& Delta);

	/** Sticks to nearby floor, or applies VerticalVelocity under gravity when unsupported. */
	void UpdateGroundedVertical(float DeltaTime);

	/**
	 * Publishes the distance actually covered this frame as Velocity, which is what
	 * APawn::GetVelocity() reads. Reporting travelled distance rather than intent means a monster
	 * pressed against a wall reads as standing still. Call once at the end of every owner tick,
	 * including the paths that moved nothing, so a held position clears the previous frame.
	 */
	void PublishFrameVelocity(float DeltaTime);

	/** Drops all motion so a pooled monster never inherits the previous life's fall or slide. */
	void ResetMotion();

protected:
	/** Downward acceleration (cm/s^2) while no floor is within snap range. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float Gravity = 980.0f;

	/** Starts the floor probe this far above the capsule center so slopes/steps are not missed. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float FloorTraceUpOffset = 50.0f;

	/** How far below the capsule center the floor probe searches. */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float FloorTraceDownDistance = 1000.0f;

	/**
	 * Max upward Z correction per update, and the tallest step MoveHorizontal will climb when a
	 * near-vertical hit might be a stair riser. Beyond this the probe/step is ignored.
	 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxFloorSnapUp = 100.0f;

	/**
	 * Max downward Z stick while walking. Farther drops use gravity instead of teleporting
	 * to the valley floor.
	 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MaxFloorSnapDown = 100.0f;

	/**
	 * Minimum surface Normal.Z to treat a blocked sweep as walkable terrain to slide along
	 * rather than a wall to stop at. ~0.7 matches CMC's default ~44-degree walkable angle.
	 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float WalkableNormalZ = 0.7f;

	/**
	 * Kept hovering this far above the floor surface (never touching it exactly). Without this,
	 * the capsule starts every horizontal sweep already in contact with the ground, and UE
	 * reports that as an immediate blocking hit — freezing movement even on flat terrain.
	 */
	UPROPERTY(EditAnywhere, Category = "Movement")
	float FloorClearance = 2.0f;

private:
	/**
	 * Line-traces for WorldStatic under Location (capsule center).
	 * On hit, OutCapsuleCenterZ is ImpactPoint.Z + scaled half-height.
	 */
	bool TryFindFloorZ(const FVector& Location, float& OutCapsuleCenterZ) const;

	/** Sweeps the owner by Delta, recording what actually moved for PublishFrameVelocity(). */
	void MoveOwner(const FVector& Delta, FHitResult* OutHit = nullptr);

	/** Places the owner without a sweep, recording the shift for PublishFrameVelocity(). */
	void SetOwnerLocation(const FVector& NewLocation);

	/** Cached from UpdatedPrimitive; floor probes need the capsule half-height and radius. */
	UPROPERTY(Transient)
	TObjectPtr<UCapsuleComponent> CapsuleComp;

	float VerticalVelocity = 0.0f;

	/** Distance moved since the last PublishFrameVelocity(). */
	FVector FrameMove = FVector::ZeroVector;


// Knockback
public:
	/** Pushes the owner away from AwayFromLocation. No-op while KnockbackSpeed is zero. */
	void BeginKnockback(const FVector& AwayFromLocation);

	/** Advances the knockback slide. Returns true while it is still moving the owner. */
	bool UpdateKnockback(float DeltaTime);

	/** True while a knockback slide still has distance left to cover. */
	FORCEINLINE bool HasActiveKnockback() const { return !KnockbackVelocity.IsNearlyZero(); }

protected:
	/** Initial push speed away from the chase target, in cm/s. Zero disables knockback. */
	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackSpeed = 400.0f;

	/** Higher values bleed the knockback off faster. */
	UPROPERTY(EditAnywhere, Category = "Knockback")
	float KnockbackDamping = 8.0f;

private:
	FVector KnockbackVelocity = FVector::ZeroVector;

};
