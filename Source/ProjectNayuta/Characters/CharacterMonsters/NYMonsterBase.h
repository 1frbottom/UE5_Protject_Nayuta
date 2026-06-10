// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NYMonsterBase.generated.h"



class UCapsuleComponent;
class USkeletalMeshComponent;
class UWidgetComponent;
class UNYHpBarWidgetMonster;

/** Replicated active/inactive state for pooled monsters (target + spawn snap). */
USTRUCT()
struct FMonsterActivationData
{
    GENERATED_BODY()

    UPROPERTY()
    TObjectPtr<AActor> Target;

    UPROPERTY()
    FVector SpawnLocation;
};

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

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
    TObjectPtr<class UStaticMeshComponent> SphereComp;

// Stat
public:
    FORCEINLINE FName GetRewardRowID() const { return RewardRowID; }

protected:
    UPROPERTY(EditAnywhere, Category = "Stat")
    float MoveSpeed = 200.0f;

    UPROPERTY()
    float RandomizedMoveSpeed;

    UPROPERTY(EditAnywhere, Category = "Stat")
    float MaxHp = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHp, EditAnywhere, Category = "Stat")
    float CurrentHp;

    /** Row name in MonsterRewardDataTable (GameMode). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    FName RewardRowID = TEXT("Default");

    UFUNCTION()
    void OnRep_CurrentHp();


// Multiplayer
public:
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    /** Authority-only: pull from pool and replicate activation to clients. */
    void ActivateOnServer(AActor* NewTarget, FVector StartLocation);
    void DeactivateOnServer();

protected:
    UPROPERTY(ReplicatedUsing = OnRep_ActivationData, Transient)
    FMonsterActivationData ActivationData;

    UFUNCTION()
    virtual void OnRep_ActivationData();



};
