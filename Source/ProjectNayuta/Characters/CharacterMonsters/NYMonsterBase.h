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

    /**
     * Rolled once on the server and replicated so every machine derives the same move speed
     * and approach angle. Drawing those locally would let the simulations drift apart.
     */
    UPROPERTY()
    uint8 MoveSeed = 0;
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

    /** Widest angle (degrees) a monster may lean off the straight line to its target. */
    UPROPERTY(EditAnywhere, Category = "Stat")
    float MaxApproachAngleOffset = 20.0f;

    UPROPERTY()
    float RandomizedMoveSpeed;

    /** Derived from ActivationData.MoveSeed; spreads the horde without desyncing machines. */
    UPROPERTY()
    float ApproachAngleOffset = 0.0f;

    UPROPERTY(EditAnywhere, Category = "Stat")
    float MaxHp = 100.0f;

    UPROPERTY(ReplicatedUsing = OnRep_CurrentHp, EditAnywhere, Category = "Stat")
    float CurrentHp = 100.0f;

    /** Row name in MonsterRewardDataTable (GameMode). */
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stat")
    FName RewardRowID = TEXT("Default");

    /**
     * Presentation hook for HP visuals (e.g. prototype SphereComp CPD).
     * Fires on every CurrentHp replication, never on a dedicated server.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Stat")
    void OnHpChanged(float NewCurrentHp, float NewMaxHp);

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

    UFUNCTION(BlueprintImplementableEvent, Category = "Death")
    void OnDeathFeedback();

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


// Hit Reaction
public:
    /** True while a recent hit is suppressing this monster's seek and attacks. */
    bool IsStaggered() const;

protected:
    /**
     * Presentation only (montage, SFX, flash), authored in Blueprint.
     * Fires on every machine that renders this monster, never on a dedicated server,
     * and is skipped for the lethal hit so the death animation owns that moment.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "Hit")
    void OnHitFeedback(float DamageTaken);

    /** Seconds the monster stops seeking and attacking after taking damage. */
    UPROPERTY(EditAnywhere, Category = "Hit")
    float StaggerDuration = 0.15f;

    /** Initial push speed away from the chase target, in cm/s. Zero disables knockback. */
    UPROPERTY(EditAnywhere, Category = "Hit")
    float KnockbackSpeed = 400.0f;

    /** Higher values bleed the knockback off faster. */
    UPROPERTY(EditAnywhere, Category = "Hit")
    float KnockbackDamping = 8.0f;

private:
    /**
     * Suppresses seek and attacks for StaggerDuration. Together with BeginKnockback() this is
     * the gameplay half of a hit: both are driven by replicated HP so server and clients start
     * them together, keeping the authoritative position close to what players see.
     */
    void BeginStagger();

    /** Pushes the monster away from its chase target. No-op without a target. */
    void BeginKnockback();

    /** Advances the knockback slide. Returns true while it is still moving the monster. */
    bool UpdateKnockback(float DeltaTime);

    /** Clears stagger and knockback so a pooled monster never inherits the previous life's state. */
    void ClearHitState();

    float StaggerEndTime = 0.0f;

    FVector KnockbackVelocity = FVector::ZeroVector;

    /** Locally cached HP so OnRep_CurrentHp can tell a hit apart from a pool refill. */
    float LastObservedHp = 0.0f;

};
