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

    virtual void Tick(float DeltaTime) override;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;


// Component
protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<UCapsuleComponent> CapsuleComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<USkeletalMeshComponent> SkeletalMeshComp;

        // test
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<class UStaticMeshComponent> SphereComp;

// Stat
public:
    virtual void ResetState();  // when pulled out of pool
    virtual void Deactivate();  // when returns to pool

protected:
    UPROPERTY(EditAnywhere, Category = "Stat")
    float MoveSpeed = 200.0f;

    UPROPERTY()
    float RandomizedMoveSpeed;

    // Hp
    UPROPERTY(EditAnywhere, Category = "Stats")
    float MaxHp = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHp, EditAnywhere, Category = "Stats")
    float CurrentHp;

    UFUNCTION()
    void OnRep_CurrentHp();


// Multiplay
public:
    // 서버에서 초기 타겟을 설정해주는 함수
    /* called by NYMonsterSpawner */
    void SetTarget(AActor* NewTarget);

    // 데미지 처리 (서버에서만 실행됨)
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
    /* initialized by NYMonsterBase->SerTarget() */
    UPROPERTY(Replicated, Transient)
    TObjectPtr<AActor> TargetActor;




};
