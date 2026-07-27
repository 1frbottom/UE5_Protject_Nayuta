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

    // Simple seek movement without navmesh.
    if (ActivationData.Target != nullptr)
    {
        FVector Direction = (ActivationData.Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        // Small random offset to reduce stacking.
        Direction.X += FMath::RandRange(-0.1f, 0.1f);
        Direction.Y += FMath::RandRange(-0.1f, 0.1f);
        Direction.Normalize();

        AddActorWorldOffset(Direction * RandomizedMoveSpeed * DeltaTime, true);

        FRotator TargetRotation = Direction.Rotation();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
    }
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
    float HpRatio = FMath::Max(0.3f, CurrentHp / MaxHp);
    
    if (SphereComp)
        SphereComp->SetCustomPrimitiveDataFloat(0, HpRatio);

    // Only a drop is a hit; a rise means the pool refilled this monster.
    // Hits landed between two net updates arrive coalesced into a single reaction.
    const float DamageTaken = LastObservedHp - CurrentHp;
    LastObservedHp = CurrentHp;

    if (DamageTaken > 0.0f && CurrentHp > 0.0f && GetNetMode() != NM_DedicatedServer)
    {
        OnHitReaction(DamageTaken);
    }

    // Runs on server and clients: a corpse keeps rendering but stops colliding and seeking.
    if (CurrentHp <= 0.0f && ActivationData.bIsActive)
    {
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

        RandomizedMoveSpeed = MoveSpeed * FMath::RandRange(0.8f, 1.2f);
        OnRep_CurrentHp();
    }
    else
    {
        // Pooled reuse: a hit reaction left mid-blend would resume on the next activation.
        if (UAnimInstance* AnimInstance = SkeletalMeshComp ? SkeletalMeshComp->GetAnimInstance() : nullptr)
        {
            AnimInstance->StopAllMontages(0.0f);
        }

        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        SetActorTickEnabled(false);
    }
}
