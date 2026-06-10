// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "ProjectNayuta.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include "Components/WidgetComponent.h"

#include "UI/NYHpBarWidgetMonster.h"

#include "Game/NYGameModeStage.h"
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

    // Debug HP visualization (replace with proper UI later).
    SphereComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HpSphereComp"));
    SphereComp->SetupAttachment(RootComponent);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void ANYMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
    CurrentHp = MaxHp;


}

void ANYMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // Simple seek movement without navmesh.
    if (ActivationData.Target != nullptr)
    {
        FVector Direction = (ActivationData.Target->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        // Small random offset to reduce stacking.
        Direction.X += FMath::RandRange(-0.1f, 0.1f);
        Direction.Y += FMath::RandRange(-0.1f, 0.1f);
        Direction.Normalize();

        AddActorWorldOffset(Direction * RandomizedMoveSpeed * DeltaTime, false);

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

float ANYMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority())
        return 0.0f;

    CurrentHp -= DamageAmount;
    OnRep_CurrentHp();

    if (CurrentHp <= 0.0f)
    {
        if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
        {
            GM->OnEnemyKilled(EventInstigator, this);

            if (GM->GetMonsterPoolComponent())
                GM->GetMonsterPoolComponent()->ReturnMonster(this);
            else
                Destroy();
        }
    }

    return DamageAmount;
}

void ANYMonsterBase::ActivateOnServer(AActor* NewTarget, FVector StartLocation)
{
    if (HasAuthority())
    {
        CurrentHp = MaxHp;

        SetActorLocation(StartLocation);
        SetNetDormancy(DORM_Awake);

        FMonsterActivationData NewData;
        NewData.Target = NewTarget;
        NewData.SpawnLocation = StartLocation;
        ActivationData = NewData;

        OnRep_ActivationData();
    }
}

void ANYMonsterBase::DeactivateOnServer()
{
    if (HasAuthority())
    {
        SetNetDormancy(DORM_DormantAll);

        FMonsterActivationData NewData;
        NewData.Target = nullptr;
        NewData.SpawnLocation = FVector::ZeroVector;
        ActivationData = NewData;

        OnRep_ActivationData();
    }
}

void ANYMonsterBase::OnRep_ActivationData()
{
    if (ActivationData.Target != nullptr)
    {
        // Apply authoritative spawn position on clients (movement is not replicated).
        SetActorLocation(ActivationData.SpawnLocation);

        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);
        SetActorTickEnabled(true);

        RandomizedMoveSpeed = MoveSpeed * FMath::RandRange(0.8f, 1.2f);
        OnRep_CurrentHp();
    }
    else
    {
        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        SetActorTickEnabled(false);
    }
}

void ANYMonsterBase::OnRep_CurrentHp()
{
    float HpRatio = FMath::Max(0.3f, CurrentHp / MaxHp);
    
    if (SphereComp)
        SphereComp->SetCustomPrimitiveDataFloat(0, HpRatio);

}
