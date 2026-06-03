// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "ProjectNayuta.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include "Components/WidgetComponent.h"

#include "UI/NYHpBarWidgetMonster.h"

#include "Game/NYGameModeStage.h"
#include "Game/NYMonsterPoolManager.h"



ANYMonsterBase::ANYMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

    // Multiplay
    bReplicates = true;

        // using client tick
    SetReplicateMovement(false);

    NetUpdateFrequency = 5.0f;
    NetCullDistanceSquared = 9000000.0f;

    NetDormancy = DORM_DormantAll;


    // Component
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    RootComponent = CapsuleComp;
    CapsuleComp->SetCollisionProfileName(PROFILE_MONSTER);
        
        // should be adjusted well
    CapsuleComp->InitCapsuleSize(15.0f, 40.0f);

        // 메시 충돌연산 제외
    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
    SkeletalMeshComp->SetupAttachment(RootComponent);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

        // test
    SphereComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HpSphereComp"));
    SphereComp->SetupAttachment(RootComponent);
    SphereComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);


}

void ANYMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
    // Stat
    CurrentHp = MaxHp;


}

void ANYMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // Navmesh 대신 벡터이동
    if (LocalTargetActor != nullptr)
    {
        FVector Direction = (LocalTargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        // 뭉침 해소 위해 벡터에 랜덤 노이즈 추가
        Direction.X += FMath::RandRange(-0.1f, 0.1f);
        Direction.Y += FMath::RandRange(-0.1f, 0.1f);
        Direction.Normalize();

        AddActorWorldOffset(Direction * RandomizedMoveSpeed * DeltaTime, false);

        // 부드러운 시선 회전 보간
        FRotator TargetRotation = Direction.Rotation();
        SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 5.0f));
    }
}

// 서버에서 스폰 직후 호출
void ANYMonsterBase::SetTarget(AActor* NewTarget)
{
    if (HasAuthority())
    {
        TargetActor = NewTarget;
        OnRep_TargetActor();
    }

}

void ANYMonsterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANYMonsterBase, TargetActor);
    DOREPLIFETIME(ANYMonsterBase, CurrentHp);
}

// 언리얼 내장 데미지 처리 함수 오버라이드
float ANYMonsterBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority())
        return 0.0f;

    CurrentHp -= DamageAmount;
    OnRep_CurrentHp();

    if (CurrentHp <= 0.0f)
    {
        // TODO : 경험치 보석 스폰 로직


        if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
        {
            GM->OnEnemyKilled();

            // 풀에 반납
            if (GM->GetMonsterPoolManager())
                GM->GetMonsterPoolManager()->ReturnMonster(this);
            else
                Destroy(); // Fallback
        }
    }

    return DamageAmount;
}

void ANYMonsterBase::ActivateOnServer(AActor* NewTarget, FVector StartLocation)
{
    if (HasAuthority())
    {
        SetActorLocation(StartLocation);
        SetNetDormancy(DORM_Awake);
        TargetActor = NewTarget;

        OnRep_TargetActor();
    }
}

void ANYMonsterBase::DeactivateOnServer()
{
    if (HasAuthority())
    {
        SetNetDormancy(DORM_DormantAll);
        TargetActor = nullptr;

        OnRep_TargetActor();
    }
}

void ANYMonsterBase::OnRep_TargetActor()
{
    if (TargetActor != nullptr)
    {
        LocalTargetActor = TargetActor;

        SetActorHiddenInGame(false);
        SetActorEnableCollision(true);
        SetActorTickEnabled(true);

        // 클라이언트 전용 속도 오차 적용 등
        RandomizedMoveSpeed = MoveSpeed * FMath::RandRange(0.8f, 1.2f);
        OnRep_CurrentHp();
    }
    else
    {
        LocalTargetActor = nullptr;

        SetActorHiddenInGame(true);
        SetActorEnableCollision(false);
        SetActorTickEnabled(false);
    }
}

void ANYMonsterBase::OnRep_CurrentHp()
{
    // test
    float HpRatio = FMath::Max(0.3f, CurrentHp / MaxHp);
    
    if (SphereComp)
        SphereComp->SetCustomPrimitiveDataFloat(0, HpRatio);

}