// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterMovementComponent.h"

#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"



namespace
{
    /** Knockback below this speed (cm/s) is snapped to zero so the tick can go back to sleep. */
    constexpr float KnockbackRestSpeed = 5.0f;

    /** Pushes the step probe this far past the capsule surface so it clears the blocking face. */
    constexpr float StepProbeForwardOffset = 5.0f;
}

UNYMonsterMovementComponent::UNYMonsterMovementComponent()
{
    // ANYMonsterBase::RefreshTickState() is the single owner of when a monster updates. Leaving
    // UMovementComponent's auto tick registration on would start a second, ungated tick for
    // every pooled monster, which is exactly the cost that tick gating exists to avoid.
    PrimaryComponentTick.bCanEverTick = false;
    bAutoUpdateTickRegistration = false;

    // Still let the base bind UpdatedComponent to the owner's root capsule on registration.
    bAutoRegisterUpdatedComponent = true;
}

void UNYMonsterMovementComponent::SetUpdatedComponent(USceneComponent* NewUpdatedComponent)
{
    Super::SetUpdatedComponent(NewUpdatedComponent);

    CapsuleComp = Cast<UCapsuleComponent>(UpdatedPrimitive);
}


// Movement
FVector UNYMonsterMovementComponent::SnapLocationToFloor(const FVector& Location) const
{
    float FloorCapsuleZ = Location.Z;
    if (TryFindFloorZ(Location, FloorCapsuleZ))
    {
        return FVector(Location.X, Location.Y, FloorCapsuleZ);
    }

    return Location;
}

void UNYMonsterMovementComponent::MoveHorizontal(const FVector& Delta)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    FHitResult Hit;
    MoveOwner(Delta, &Hit);

    if (!Hit.bBlockingHit)
    {
        return;
    }

    const FVector Remaining = Delta * (1.0f - Hit.Time);

    if (Hit.Normal.Z >= WalkableNormalZ)
    {
        // Slope, not a wall (real walls have a near-vertical Normal.Z) — slide the unswept
        // remainder along the surface instead of stopping dead.
        MoveOwner(FVector::VectorPlaneProject(Remaining, Hit.Normal));
        return;
    }

    // Near-vertical face: may be a stair riser rather than a wall. Measure the floor just past
    // the face and lift exactly that far. Lifting a fixed MaxFloorSnapUp instead would leave the
    // capsule airborne on shallow steps, and UpdateGroundedVertical would drop it back down —
    // that round trip is the visible pop when climbing.
    const FVector StepDirection = Delta.GetSafeNormal2D();
    if (!CapsuleComp || StepDirection.IsNearlyZero() || Remaining.IsNearlyZero())
    {
        return;
    }

    const FVector Location = Owner->GetActorLocation();
    const float ProbeDistance = CapsuleComp->GetScaledCapsuleRadius() + StepProbeForwardOffset;

    // Probed from a full step above so any tread within MaxFloorSnapUp is inside the trace range.
    // An overhang there reads as an out-of-range step, which leaves the monster blocked.
    FVector ProbeLocation = Location + StepDirection * ProbeDistance;
    ProbeLocation.Z += MaxFloorSnapUp;

    float StepCapsuleZ = 0.0f;
    if (!TryFindFloorZ(ProbeLocation, StepCapsuleZ))
    {
        return;
    }

    const float StepUpHeight = StepCapsuleZ - Location.Z;
    if (StepUpHeight <= 0.0f || StepUpHeight > MaxFloorSnapUp)
    {
        return;
    }

    MoveOwner(FVector(0.0f, 0.0f, StepUpHeight));

    // The sweep above may have been cut short by a ceiling, so undo by what actually moved.
    const float LiftedZ = Owner->GetActorLocation().Z;

    FHitResult StepHit;
    MoveOwner(Remaining, &StepHit);

    if (StepHit.bBlockingHit)
    {
        MoveOwner(FVector(0.0f, 0.0f, Location.Z - LiftedZ));
    }
}

void UNYMonsterMovementComponent::UpdateGroundedVertical(float DeltaTime)
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    const FVector Location = Owner->GetActorLocation();

    float FloorCapsuleZ = Location.Z;
    if (TryFindFloorZ(Location, FloorCapsuleZ))
    {
        const float DeltaZ = FloorCapsuleZ - Location.Z;

        // Floor is in range : snap
        if (DeltaZ <= MaxFloorSnapUp && DeltaZ >= -MaxFloorSnapDown)
        {
            if (!FMath::IsNearlyEqual(Location.Z, FloorCapsuleZ, 0.1f))
            {
                SetOwnerLocation(FVector(Location.X, Location.Y, FloorCapsuleZ));
            }
            VerticalVelocity = 0.0f;
            return;
        }

        // Floor is far below : fall, then catch when the capsule reaches it
        // this condition is excuted only when end of falling (to stop by destination Z)
        if (DeltaZ < -MaxFloorSnapDown && Location.Z + VerticalVelocity * DeltaTime <= FloorCapsuleZ)
        {
            SetOwnerLocation(FVector(Location.X, Location.Y, FloorCapsuleZ));
            VerticalVelocity = 0.0f;
            return;
        }
    }

    VerticalVelocity -= Gravity * DeltaTime;
    MoveOwner(FVector(0.0f, 0.0f, VerticalVelocity * DeltaTime));

    const FVector FallenLocation = Owner->GetActorLocation();
    if (TryFindFloorZ(FallenLocation, FloorCapsuleZ) && FallenLocation.Z <= FloorCapsuleZ)
    {
        SetOwnerLocation(FVector(FallenLocation.X, FallenLocation.Y, FloorCapsuleZ));
        VerticalVelocity = 0.0f;
    }
}

void UNYMonsterMovementComponent::PublishFrameVelocity(float DeltaTime)
{
    Velocity = (DeltaTime > 0.0f) ? FrameMove / DeltaTime : FVector::ZeroVector;
    FrameMove = FVector::ZeroVector;
}

void UNYMonsterMovementComponent::ResetMotion()
{
    VerticalVelocity = 0.0f;
    KnockbackVelocity = FVector::ZeroVector;
    FrameMove = FVector::ZeroVector;
    Velocity = FVector::ZeroVector;
}

bool UNYMonsterMovementComponent::TryFindFloorZ(const FVector& Location, float& OutCapsuleCenterZ) const
{
    const UWorld* World = GetWorld();
    const AActor* Owner = GetOwner();
    if (!World || !Owner || !CapsuleComp)
    {
        return false;
    }

    const FVector TraceStart(Location.X, Location.Y, Location.Z + FloorTraceUpOffset);
    const FVector TraceEnd(Location.X, Location.Y, Location.Z - FloorTraceDownDistance);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterFloor), false, Owner);
    FHitResult Hit;
    // ByObjectType(WorldStatic) only, so other monsters/players (unset Visibility response
    // defaults to Block on their profiles) can never be mistaken for the floor.
    if (!World->LineTraceSingleByObjectType(Hit, TraceStart, TraceEnd, FCollisionObjectQueryParams(ECC_WorldStatic), Params))
    {
        return false;
    }

    // +FloorClearance: never let the capsule rest exactly on the surface (see FloorClearance).
    OutCapsuleCenterZ = Hit.ImpactPoint.Z + CapsuleComp->GetScaledCapsuleHalfHeight() + FloorClearance;
    return true;
}

void UNYMonsterMovementComponent::MoveOwner(const FVector& Delta, FHitResult* OutHit)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    const FVector BeforeLocation = Owner->GetActorLocation();
    Owner->AddActorWorldOffset(Delta, true, OutHit);
    FrameMove += Owner->GetActorLocation() - BeforeLocation;
}

void UNYMonsterMovementComponent::SetOwnerLocation(const FVector& NewLocation)
{
    AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    const FVector BeforeLocation = Owner->GetActorLocation();
    Owner->SetActorLocation(NewLocation);
    FrameMove += Owner->GetActorLocation() - BeforeLocation;
}


// Knockback
void UNYMonsterMovementComponent::BeginKnockback(const FVector& AwayFromLocation)
{
    const AActor* Owner = GetOwner();
    if (!Owner)
    {
        return;
    }

    const FVector Away = (Owner->GetActorLocation() - AwayFromLocation).GetSafeNormal2D();
    KnockbackVelocity = Away * KnockbackSpeed;
}

bool UNYMonsterMovementComponent::UpdateKnockback(float DeltaTime)
{
    if (KnockbackVelocity.IsNearlyZero())
    {
        return false;
    }

    MoveOwner(KnockbackVelocity * DeltaTime);
    KnockbackVelocity = FMath::VInterpTo(KnockbackVelocity, FVector::ZeroVector, DeltaTime, KnockbackDamping);

    // Stop chasing an ever-smaller remainder.
    if (KnockbackVelocity.SizeSquared() < FMath::Square(KnockbackRestSpeed))
    {
        KnockbackVelocity = FVector::ZeroVector;
        return false;
    }

    return true;
}
