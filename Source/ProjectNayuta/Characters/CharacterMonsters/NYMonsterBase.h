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

    /** When true, monster is visible/collidable. Target may still be null (idle). */
    UPROPERTY()
    bool bIsActive = false;

    UPROPERTY()
    TObjectPtr<AActor> Target;

    UPROPERTY()
    FVector SpawnLocation = FVector::ZeroVector;
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
public:
    FORCEINLINE UCapsuleComponent* GetCapsuleComponent() const { return CapsuleComp; }

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
    FORCEINLINE float GetMaxHp() const { return MaxHp; }
    FORCEINLINE float GetCurrentHp() const { return CurrentHp; }

    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    /** Authority-only: set MaxHp and refill CurrentHp. */
    void SetMaxHpOnServer(float NewMaxHp);

protected:
    UPROPERTY(EditAnywhere, Category = "Stat")
    float MoveSpeed = 200.0f;

    UPROPERTY()
    float RandomizedMoveSpeed;

    UPROPERTY(EditAnywhere, Category = "Stat")
    float MaxHp = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHp, EditAnywhere, Category = "Stat")
    float CurrentHp = 100.0f;

    /** Row name in MonsterRewardDataTable (GameMode). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    FName RewardRowID = TEXT("Default");

    UFUNCTION()
    void OnRep_CurrentHp();


// Death
public:
    FORCEINLINE bool IsDying() const { return bIsDying; }

protected:
    /** Authority-only: grant rewards, stop acting, and return to the pool after the death animation. */
    void StartDeathOnServer(AController* KillerController);

    /** Authority-only: hand the corpse back to the pool (or destroy it when no pool exists). */
    void FinishDeathOnServer();

    /** Authority-only: stop attack timers. Overridden by monsters that own one. */
    virtual void StopAttackOnServer() {}

    /** Seconds the corpse stays visible so every machine can play the death animation. */
    UPROPERTY(EditAnywhere, Category = "Death")
    float DeathDuration = 2.0f;

    /** Server-only. Clients infer the corpse state from replicated CurrentHp. */
    UPROPERTY(Transient)
    bool bIsDying = false;

private:
    FTimerHandle DeathTimerHandle;


// Multiplay
public:
    FORCEINLINE bool IsActive() const { return ActivationData.bIsActive; }

    /** Authority-only: show at location and chase NewTarget (pooled combat spawn). */
    void ActivateOnServer(AActor* NewTarget, FVector StartLocation);

    /** Authority-only: show at location with no chase target (training idle). */
    void ActivateIdleOnServer(FVector StartLocation);

    void DeactivateOnServer();  

protected:
    UPROPERTY(ReplicatedUsing = OnRep_ActivationData, Transient)
    FMonsterActivationData ActivationData;

    UFUNCTION()
    virtual void OnRep_ActivationData();

};
