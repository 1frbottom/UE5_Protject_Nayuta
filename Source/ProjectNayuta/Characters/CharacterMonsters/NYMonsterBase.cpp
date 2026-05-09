// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "ProjectNayuta.h"

#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"

#include "Components/WidgetComponent.h"

#include "UI/NYHpBarWidgetMonster.h"

#include "Game/NYGameMode.h"



ANYMonsterBase::ANYMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

    // Multiplay
    bReplicates = true;
    SetReplicateMovement(true);

    // Component
    CapsuleComp = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComp"));
    RootComponent = CapsuleComp;
    CapsuleComp->SetCollisionProfileName(PROFILE_MONSTER);

        // UI
    HpBarWidgetComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HpBarWidgetComp"));
    HpBarWidgetComp->SetupAttachment(RootComponent);
    HpBarWidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
    HpBarWidgetComp->SetDrawSize(FVector2D(100.0f, 15.0f));
    HpBarWidgetComp->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

        // 메시 충돌연산 제외
    SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
    SkeletalMeshComp->SetupAttachment(RootComponent);
    SkeletalMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);



}

void ANYMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	
    // Stat
    CurrentHp = MaxHp;

    // UI
    if (HpBarWidgetComp)
    {
        // 캐스팅은 딱 한번만 하도록 BeginPlay에서
        CachedHpBarWidget = Cast<UNYHpBarWidgetMonster>(HpBarWidgetComp->GetUserWidgetObject());

        // 스폰 직후 풀피 상태 UI 반영
        if (CachedHpBarWidget)
        {
            CachedHpBarWidget->UpdateHpBar(1.0f);
        }
    }


}

void ANYMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    // Navmesh 대신 벡터이동
    if (TargetActor != nullptr)
    {
        FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

        // Sweep을 true로 주어 몬스터끼리 겹치는 것 방지
        AddActorWorldOffset(Direction * MoveSpeed * DeltaTime, true);

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
    // 체력 깎기는 무조건 서버에서만
    if (!HasAuthority())
        return 0.0f;

    CurrentHp -= DamageAmount;

    OnRep_CurrentHp();

    if (CurrentHp <= 0.0f)
    {
        // TODO : 경험치 보석 스폰 로직 호출
        

        if (ANYGameMode* GM = Cast<ANYGameMode>(GetWorld()->GetAuthGameMode()))
        {
            GM->OnEnemyKilled();
        }
        
        Destroy();
    }

    return DamageAmount;
}

void ANYMonsterBase::OnRep_CurrentHp()
{
    // 캐싱된 포인터를 사용해 형변환 없이 즉시 UI 갱신
    if (CachedHpBarWidget)
    {
        float HpPercentage = CurrentHp / MaxHp;
        CachedHpBarWidget->UpdateHpBar(HpPercentage);
    }
}