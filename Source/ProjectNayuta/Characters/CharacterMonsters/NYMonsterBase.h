// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NYMonsterBase.generated.h"

class UCapsuleComponent;
class USkeletalMeshComponent;
class UWidgetComponent;
class UNYHpBarWidgetMonster;

UCLASS()
class PROJECTNAYUTA_API ANYMonsterBase : public APawn
{
	GENERATED_BODY()

public:
	ANYMonsterBase();

protected:
	virtual void BeginPlay() override;

// Component
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<UCapsuleComponent> CapsuleComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

// Stat
protected:
    UPROPERTY(EditAnywhere, Category = "Stat")
    float MoveSpeed = 200.0f;

    // Hp
    UPROPERTY(EditAnywhere, Category = "Stats")
    float MaxHp = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHp, EditAnywhere, Category = "Stats")
    float CurrentHp;

    UFUNCTION()
    void OnRep_CurrentHp();

// UI
protected:
    /* Must be designated in BP_MonsterASDF*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UWidgetComponent> HpBarWidgetComp;

    UPROPERTY()
    TObjectPtr<UNYHpBarWidgetMonster> CachedHpBarWidget;

// Multiplay
protected:
    /* initialized by NYMonsterBase->SerTarget() */
    UPROPERTY(Replicated, Transient)
    TObjectPtr<AActor> TargetActor;

public:	
	virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    // 서버에서 초기 타겟을 설정해주는 함수
    /* called by NYMonsterSpawner */
    void SetTarget(AActor* NewTarget);

    // 데미지 처리 (서버에서만 실행됨)
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;


};
