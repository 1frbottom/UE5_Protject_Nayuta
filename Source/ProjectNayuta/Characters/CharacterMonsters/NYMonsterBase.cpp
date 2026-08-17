// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "ProjectNayuta.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "CollisionQueryParams.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"
#include "GameFramework/GameModeBase.h"
#include "Net/UnrealNetwork.h"

#include "Game/NYMonsterLifecycleInterface.h"



namespace
{
    /** Knockback below this speed (cm/s) is snapped to zero so the tick can go back to sleep. */
    constexpr float KnockbackRestSpeed = 5.0f;

    /** Pushes the step probe this far past the capsule surface so it clears the blocking face. */
    constexpr float StepProbeForwardOffset = 5.0f;
}

ANYMonsterBase::ANYMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

    bReplicates = true;

    // Movement is simulated on each machine; ActivationData carries spawn position to clients.
    SetReplicateMovement(false);

    NetUpdateFrequency = 5.0f;
    NetCullDistanceSquared = 9000000.0f;

    NetDormancy = DORM_DormantAll;


    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    RootComponent = CapsuleComp;
    CapsuleComp->SetCollisionProfileName(PROFILE_MONSTER);
    CapsuleComp->InitCapsuleSize(15.0f, 40.0f);

    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
    SkeletalMeshComp->SetupAttachment(RootComponent);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Horde scale: offscreen monsters skip pose evaluation entirely.
    SkeletalMeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;

    // Debug HP visualization (replace with proper UI later).
    SphereComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HpSphereComp"));
    SphereComp->SetupAttachment(RootComponent);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void ANYMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CurrentHp <= 0.0f)
	{
		return;
	}

    const bool bKnockbackActive = UpdateKnockback(DeltaTime);

    // Idle monsters keep their tick off; a hit turns it back on just long enough to slide.
    if (ActivationData.Target == nullptr)
    {
        if (bKnockbackActive)
        {
            UpdateGroundedVertical(DeltaTime);
        }
        else
        {
            // UpdateKnockback() zeroed the velocity, so the monster goes back to sleep here.
            RefreshTickState();
        }
        return;
    }

    if (!IsStaggered())
    {
        // Horizontal seek only — vertical is owned by UpdateGroundedVertical (no navmesh).
        const FVector ToTarget = ActivationData.Target->GetActorLocation() - GetActorLocation();
        FVector Direction = ToTarget.GetSafeNormal2D();

        // Constant per-monster lean off the straight line, so the horde spreads instead of
        // funnelling into one point. Derived from the replicated seed, unlike a per-frame draw.
        Direction = Direction.RotateAngleAxis(ApproachAngleOffset, FVector::UpVector);

        // Position holds still once within attack range (or mid-attack); facing keeps tracking the target.
        // Without this, a ranged monster would keep closing to melee distance between attacks.
        const bool bWithinAttackRange = ToTarget.SizeSquared2D() <= FMath::Square(AttackRange);
        if (!IsAttacking() && !bWithinAttackRange && !Direction.IsNearlyZero())
        {
            MoveHorizontal(Direction * RandomizedMoveSpeed * DeltaTime);
        }

        if (!Direction.IsNearlyZero())
        {
            const FRotator TargetRotation = Direction.Rotation();
            SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
        }
    }

    UpdateGroundedVertical(DeltaTime);
}

void ANYMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANYMonsterBase, ActivationData);
    DOREPLIFETIME(ANYMonsterBase, CurrentHp);
}

void ANYMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
    CurrentHp = MaxHp;
    LastObservedHp = CurrentHp;

    // A shorter interval would cut the swing off before the longest variant finishes playing.
    for (const UAnimMontage* Montage : AttackMontages)
    {
        if (Montage)
        {
            AttackInterval = FMath::Max(AttackInterval, Montage->GetPlayLength());
        }
    }
}

bool ANYMonsterBase::ShouldTick() const
{
    // A pooled or dead monster has nothing to simulate; a corpse still renders its death animation.
    if (!ActivationData.bIsActive || CurrentHp <= 0.0f)
    {
        return false;
    }

    // Seeking runs per frame. An idle monster only wakes up for the length of a knockback slide.
    return ActivationData.Target != nullptr || !KnockbackVelocity.IsNearlyZero();
}

void ANYMonsterBase::RefreshTickState()
{
    SetActorTickEnabled(ShouldTick());
}

bool ANYMonsterBase::TryFindFloorZ(const FVector& Location, float& OutCapsuleCenterZ) const
{
    const UWorld* World = GetWorld();
    if (!World || !CapsuleComp)
    {
        return false;
    }

    const FVector TraceStart(Location.X, Location.Y, Location.Z + FloorTraceUpOffset);
    const FVector TraceEnd(Location.X, Location.Y, Location.Z - FloorTraceDownDistance);

    FCollisionQueryParams Params(SCENE_QUERY_STAT(MonsterFloor), false, this);
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

FVector ANYMonsterBase::SnapLocationToFloor(const FVector& Location) const
{
    float FloorCapsuleZ = Location.Z;
    if (TryFindFloorZ(Location, FloorCapsuleZ))
    {
        return FVector(Location.X, Location.Y, FloorCapsuleZ);
    }

    return Location;
}

void ANYMonsterBase::MoveHorizontal(const FVector& Delta)
{
    FHitResult Hit;
    AddActorWorldOffset(Delta, true, &Hit);

    if (!Hit.bBlockingHit)
    {
        return;
    }

    const FVector Remaining = Delta * (1.0f - Hit.Time);

    if (Hit.Normal.Z >= WalkableNormalZ)
    {
        // Slope, not a wall (real walls have a near-vertical Normal.Z) — slide the unswept
        // remainder along the surface instead of stopping dead. No AActor-level movement
        // component gives us this for free, so it is projected by hand.
        AddActorWorldOffset(FVector::VectorPlaneProject(Remaining, Hit.Normal), true);
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

    const FVector Location = GetActorLocation();
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

    AddActorWorldOffset(FVector(0.0f, 0.0f, StepUpHeight), true);

    // The sweep above may have been cut short by a ceiling, so undo by what actually moved.
    const float LiftedZ = GetActorLocation().Z;

    FHitResult StepHit;
    AddActorWorldOffset(Remaining, true, &StepHit);

    if (StepHit.bBlockingHit)
    {
        AddActorWorldOffset(FVector(0.0f, 0.0f, Location.Z - LiftedZ), true);
    }
}

void ANYMonsterBase::UpdateGroundedVertical(float DeltaTime)
{
    const FVector Location = GetActorLocation();

    float FloorCapsuleZ = Location.Z;
    if (TryFindFloorZ(Location, FloorCapsuleZ))
    {
        const float DeltaZ = FloorCapsuleZ - Location.Z;

        // Floor is in range : snap
        if (DeltaZ <= MaxFloorSnapUp && DeltaZ >= -MaxFloorSnapDown)
        {
            if (!FMath::IsNearlyEqual(Location.Z, FloorCapsuleZ, 0.1f))
            {
                SetActorLocation(FVector(Location.X, Location.Y, FloorCapsuleZ));
            }
            VerticalVelocity = 0.0f;
            return;
        }

        // Floor is far below : fall, then catch when the capsule reaches it
        // this condition is excuted only when end of falling (to stop by destination Z)
        if (DeltaZ < -MaxFloorSnapDown && Location.Z + VerticalVelocity * DeltaTime <= FloorCapsuleZ)
        {
            SetActorLocation(FVector(Location.X, Location.Y, FloorCapsuleZ));
            VerticalVelocity = 0.0f;
            return;
        }
    }

    VerticalVelocity -= Gravity * DeltaTime;
    AddActorWorldOffset(FVector(0.0f, 0.0f, VerticalVelocity * DeltaTime), true);

    const FVector FallenLocation = GetActorLocation();
    if (TryFindFloorZ(FallenLocation, FloorCapsuleZ) && FallenLocation.Z <= FloorCapsuleZ)
    {
        SetActorLocation(FVector(FallenLocation.X, FallenLocation.Y, FloorCapsuleZ));
        VerticalVelocity = 0.0f;
    }
}


// Stat
float ANYMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority())
        return 0.0f;

    // Corpses are still visible during the death animation, so reject damage that would kill twice.
    if (bIsDying)
        return 0.0f;

    CurrentHp -= DamageAmount;
    OnRep_CurrentHp();

    if (CurrentHp <= 0.0f)
    {
        StartDeathOnServer(EventInstigator);
    }

    return DamageAmount;
}

void ANYMonsterBase::SetMaxHpOnServer(float NewMaxHp)
{
    if (!HasAuthority())
    {
        return;
    }

    MaxHp = FMath::Max(1.0f, NewMaxHp);
    CurrentHp = MaxHp;
    OnRep_CurrentHp();
}

void ANYMonsterBase::OnRep_CurrentHp()
{
    if (GetNetMode() != NM_DedicatedServer)
    {
        OnHpChanged(CurrentHp, MaxHp);
    }

    // Only a drop is a hit; a rise means the pool refilled this monster.
    // Hits landed between two net updates arrive coalesced into a single reaction.
    const float DamageTaken = LastObservedHp - CurrentHp;
    LastObservedHp = CurrentHp;

    if (DamageTaken > 0.0f)
    {
        // Stagger/knockback are for living monsters. A corpse should not slide.
        if (CurrentHp > 0.0f)
        {
            // Gameplay runs everywhere, including a dedicated server, so the authoritative
            // position stays in step with what players are looking at.
            BeginStagger();
            BeginKnockback();
        }

        if (GetNetMode() != NM_DedicatedServer)
        {
            OnHitFeedback(DamageTaken);
        }
    }

    // Runs on server and clients: a corpse keeps rendering but stops colliding and seeking.
    if (CurrentHp <= 0.0f && ActivationData.bIsActive)
    {
        // Attack (and the hurt montage from OnHitFeedback) occupy the slot until stopped.
        // Without this, AnimGraph death waits for the attack montage to finish.
        if (UAnimInstance* AnimInstance = SkeletalMeshComp ? SkeletalMeshComp->GetAnimInstance() : nullptr)
        {
            AnimInstance->StopAllMontages(0.15f);
        }

        if (GetNetMode() != NM_DedicatedServer)
        {
            OnDeathFeedback();
        }

        SetActorEnableCollision(false);
        RefreshTickState();
    }
}


// Death
void ANYMonsterBase::StartDeathOnServer(AController* KillerController)
{
    if (!HasAuthority() || bIsDying)
    {
        return;
    }

    bIsDying = true;
    StopAttackOnServer();

    // Kill count and rewards are granted immediately; only the cleanup waits for the animation.
    if (INYMonsterLifecycleInterface* LifecycleHost = GetLifecycleHost())
    {
        LifecycleHost->NotifyMonsterKilled(KillerController, this);
    }

    // NotifyMonsterKilled can end the wave, which already returned every monster to the pool.
    if (!ActivationData.bIsActive)
    {
        bIsDying = false;
        return;
    }

    if (DeathDuration <= 0.0f)
    {
        FinishDeathOnServer();
        return;
    }

    GetWorldTimerManager().SetTimer(DeathTimerHandle, this, &ANYMonsterBase::FinishDeathOnServer, DeathDuration, false);
}

void ANYMonsterBase::StopAttackOnServer()
{
    // Server-only: OnRep_ActivationData only ever sets this timer under HasAuthority().
    GetWorldTimerManager().ClearTimer(AttackTimerHandle);
}

void ANYMonsterBase::FinishDeathOnServer()
{
    if (!HasAuthority() || !bIsDying)
    {
        return;
    }

    bIsDying = false;

    // What a corpse becomes is the host's call: Stage pools it, Training resets it in place.
    // Only when nobody claims it does the monster clean itself up.
    if (INYMonsterLifecycleInterface* LifecycleHost = GetLifecycleHost())
    {
        if (LifecycleHost->ReclaimMonster(this))
        {
            return;
        }
    }

    Destroy();
}

INYMonsterLifecycleInterface* ANYMonsterBase::GetLifecycleHost() const
{
    const UWorld* World = GetWorld();

    return World ? Cast<INYMonsterLifecycleInterface>(World->GetAuthGameMode()) : nullptr;
}


// Multiplay
void ANYMonsterBase::ActivateOnServer(AActor* NewTarget, FVector StartLocation)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentHp = MaxHp;

    // Server snaps once; clients receive the floored Z via ActivationData.SpawnLocation.
    const FVector SnappedLocation = SnapLocationToFloor(StartLocation);
    SetActorLocation(SnappedLocation);
    SetNetDormancy(DORM_Awake);

    FMonsterActivationData NewData;
    NewData.bIsActive = true;
    NewData.Target = NewTarget;
    NewData.SpawnLocation = SnappedLocation;
    NewData.MoveSeed = static_cast<uint8>(FMath::RandRange(0, 255));
    ActivationData = NewData;

    OnRep_ActivationData();
}

void ANYMonsterBase::DeactivateOnServer()
{
    if (!HasAuthority())
    {
        return;
    }

    // A wave can end mid-death; drop the pending return so the monster is not pooled twice.
    GetWorldTimerManager().ClearTimer(DeathTimerHandle);
    bIsDying = false;

    SetNetDormancy(DORM_DormantAll);

    FMonsterActivationData NewData;
    NewData.bIsActive = false;
    NewData.Target = nullptr;
    NewData.SpawnLocation = FVector::ZeroVector;
    ActivationData = NewData;

    OnRep_ActivationData();
}

void ANYMonsterBase::OnRep_ActivationData()
{
    if (ActivationData.bIsActive)
    {
        // Apply authoritative spawn position on clients (movement is not replicated).
        SetActorLocation(ActivationData.SpawnLocation);
        VerticalVelocity = 0.0f;

        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);

        ClearHitState();

        // Both values come from the replicated seed, so every machine walks this monster
        // the same way. The golden ratio keeps the angle from tracking the speed.
        const float SpeedAlpha = ActivationData.MoveSeed / 255.0f;
        const float AngleAlpha = FMath::Frac(ActivationData.MoveSeed * 0.6180339887f);

        RandomizedMoveSpeed = MoveSpeed * FMath::Lerp(0.8f, 1.2f, SpeedAlpha);
        ApproachAngleOffset = FMath::Lerp(-MaxApproachAngleOffset, MaxApproachAngleOffset, AngleAlpha);

        OnRep_CurrentHp();
    }
    else
    {
        // Pooled reuse: a hit montage left mid-blend would resume on the next activation.
        if (UAnimInstance* AnimInstance = SkeletalMeshComp ? SkeletalMeshComp->GetAnimInstance() : nullptr)
        {
            AnimInstance->StopAllMontages(0.0f);
        }

        ClearHitState();

        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
    }

    // Covers both idle-active (Target null) and deactivated monsters, matching the
    // bIsActive branches above without duplicating the null check in each of them.
    if (ActivationData.Target != nullptr)
    {
        // Server: only the authority ever ticks its own attack timer.
        if (HasAuthority())
        {
            GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ANYMonsterBase::ProcessAttack, FMath::Max(AttackInterval, 0.01f), true);
        }
    }
    else
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);
    }

    // Last, so ShouldTick() reads the state both branches above have finished settling.
    RefreshTickState();
}


// Hit Reaction
bool ANYMonsterBase::IsStaggered() const
{
    const UWorld* World = GetWorld();
    return World && World->GetTimeSeconds() < StaggerEndTime;
}

void ANYMonsterBase::BeginStagger()
{
    if (const UWorld* World = GetWorld())
    {
        StaggerEndTime = World->GetTimeSeconds() + StaggerDuration;
    }
}

void ANYMonsterBase::BeginKnockback()
{
    // The attacker is not replicated, but the chase target is. In a top-down horde the
    // player being chased is almost always the one landing the hit, so push away from them.
    const AActor* Target = ActivationData.Target;
    if (!Target)
    {
        return;
    }

    const FVector Away = (GetActorLocation() - Target->GetActorLocation()).GetSafeNormal2D();
    KnockbackVelocity = Away * KnockbackSpeed;

    // An idle monster has its tick off; the slide needs it back for a moment.
    RefreshTickState();
}

bool ANYMonsterBase::UpdateKnockback(float DeltaTime)
{
    if (KnockbackVelocity.IsNearlyZero())
    {
        return false;
    }

    AddActorWorldOffset(KnockbackVelocity * DeltaTime, true);
    KnockbackVelocity = FMath::VInterpTo(KnockbackVelocity, FVector::ZeroVector, DeltaTime, KnockbackDamping);

    // Stop chasing an ever-smaller remainder.
    if (KnockbackVelocity.SizeSquared() < FMath::Square(KnockbackRestSpeed))
    {
        KnockbackVelocity = FVector::ZeroVector;
        return false;
    }

    return true;
}

void ANYMonsterBase::ClearHitState()
{
    StaggerEndTime = 0.0f;
    KnockbackVelocity = FVector::ZeroVector;
    VerticalVelocity = 0.0f;
}

// Attack
bool ANYMonsterBase::CanAttack() const
{
    if (ActivationData.Target == nullptr)
    {
        return false;
    }

    if (IsStaggered())
    {
        return false;
    }

    const float DistSq = FVector::DistSquaredXY(GetActorLocation(), ActivationData.Target->GetActorLocation());
    return DistSq <= FMath::Square(AttackRange);
}

void ANYMonsterBase::ProcessAttack()
{
    if (CanAttack())
    {
        PerformAttack();

        UAnimMontage* ChosenMontage = AttackMontages.Num() > 0
            ? AttackMontages[FMath::RandRange(0, AttackMontages.Num() - 1)]
            : nullptr;

        Multicast_OnAttackStarted(ChosenMontage);
    }
}

bool ANYMonsterBase::IsAttacking() const
{
    const UWorld* World = GetWorld();
    
    return World && World->GetTimeSeconds() < AttackEndTime;
}

// NetMulticast
void ANYMonsterBase::Multicast_OnAttackStarted_Implementation(UAnimMontage* MontageToPlay)
{
    // Runs on every machine, including the server, so the local seek simulation freezes in step.
    if (const UWorld* World = GetWorld())
    {
        const float FreezeDuration = MontageToPlay ? MontageToPlay->GetPlayLength() : AttackFreezeDuration;
        AttackEndTime = World->GetTimeSeconds() + FreezeDuration;
    }

    // Montage playback is presentation only; a dedicated server has nothing to render.
    if (GetNetMode() == NM_DedicatedServer || !MontageToPlay)
    {
        return;
    }

    if (UAnimInstance* AnimInstance = SkeletalMeshComp ? SkeletalMeshComp->GetAnimInstance() : nullptr)
    {
        AnimInstance->Montage_Play(MontageToPlay);
    }
}
