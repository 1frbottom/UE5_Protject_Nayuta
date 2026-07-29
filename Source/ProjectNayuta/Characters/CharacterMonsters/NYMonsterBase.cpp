// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "ProjectNayuta.h"

#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include "Game/NYGameModeStage.h"
#include "Game/NYGameModeTraining.h"
#include "Game/NYMonsterPoolComponent.h"



namespace
{
    /** Knockback below this speed (cm/s) is snapped to zero so the tick can go back to sleep. */
    constexpr float KnockbackRestSpeed = 5.0f;
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
        if (!bKnockbackActive)
        {
            SetActorTickEnabled(false);
        }
        return;
    }

    if (IsStaggered())
    {
        return;
    }

    // Simple seek movement without navmesh.
    FVector Direction = (ActivationData.Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

    // Constant per-monster lean off the straight line, so the horde spreads instead of
    // funnelling into one point. Derived from the replicated seed, unlike a per-frame draw.
    Direction = Direction.RotateAngleAxis(ApproachAngleOffset, FVector::UpVector);

    AddActorWorldOffset(Direction * RandomizedMoveSpeed * DeltaTime, true);

    FRotator TargetRotation = Direction.Rotation();
    SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
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

    // The lethal hit belongs to the death animation, not to a stagger.
    if (DamageTaken > 0.0f && CurrentHp > 0.0f)
    {
        // Gameplay runs everywhere, including a dedicated server, so the authoritative
        // position stays in step with what players are looking at.
        BeginStagger();
        BeginKnockback();

        if (GetNetMode() != NM_DedicatedServer)
        {
            OnHitFeedback(DamageTaken);
        }
    }

    // Runs on server and clients: a corpse keeps rendering but stops colliding and seeking.
    if (CurrentHp <= 0.0f && ActivationData.bIsActive)
    {
        if (GetNetMode() != NM_DedicatedServer)
        {
            OnDeathFeedback();
        }

        SetActorEnableCollision(false);
        SetActorTickEnabled(false);
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
    if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnEnemyKilled(KillerController, this);
    }

    // OnEnemyKilled can end the wave, which already returned every monster to the pool.
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

void ANYMonsterBase::FinishDeathOnServer()
{
    if (!HasAuthority() || !bIsDying)
    {
        return;
    }

    bIsDying = false;

    if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
    {
        if (UNYMonsterPoolComponent* Pool = GM->GetMonsterPoolComponent())
        {
            Pool->ReturnMonster(this);
            return;
        }
    }
    else if (ANYGameModeTraining* TrainingGM = Cast<ANYGameModeTraining>(GetWorld()->GetAuthGameMode()))
    {
        // Training reuses the same actor: the reset re-activates it and refills HP.
        TrainingGM->NotifyTrainingMonsterDefeated(this);
        return;
    }

    Destroy();
}


// Multiplay
void ANYMonsterBase::ActivateOnServer(AActor* NewTarget, FVector StartLocation)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentHp = MaxHp;

    SetActorLocation(StartLocation);
    SetNetDormancy(DORM_Awake);

    FMonsterActivationData NewData;
    NewData.bIsActive = true;
    NewData.Target = NewTarget;
    NewData.SpawnLocation = StartLocation;
    NewData.MoveSeed = static_cast<uint8>(FMath::RandRange(0, 255));
    ActivationData = NewData;

    OnRep_ActivationData();
}

void ANYMonsterBase::ActivateIdleOnServer(FVector StartLocation)
{
    if (!HasAuthority())
    {
        return;
    }

    CurrentHp = MaxHp;

    SetActorLocation(StartLocation);
    SetNetDormancy(DORM_Awake);

    FMonsterActivationData NewData;
    NewData.bIsActive = true;
    NewData.Target = nullptr;
    NewData.SpawnLocation = StartLocation;
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

        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);
        // Tick only when chasing; idle stays visible without seek.
        SetActorTickEnabled(ActivationData.Target != nullptr);

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
        SetActorTickEnabled(false);
    }
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
    SetActorTickEnabled(true);
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
}
