// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "NYMonsterBase.generated.h"



class UCapsuleComponent;
class USkeletalMeshComponent;
class UWidgetComponent;
class UNYHpBarWidgetMonster;
class UAnimMontage;
class INYMonsterLifecycleInterface;

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


// Movement
protected:
    /** Downward acceleration (cm/s^2) while no floor is within snap range. */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float Gravity = 980.0f;

    /** Starts the floor probe this far above the capsule center so slopes/steps are not missed. */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float FloorTraceUpOffset = 50.0f;

    /** How far below the capsule center the floor probe searches. */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float FloorTraceDownDistance = 1000.0f;

    /**
     * Max upward Z correction per update, and the tallest step MoveHorizontal will climb when a
     * near-vertical hit might be a stair riser. Beyond this the probe/step is ignored.
     */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxFloorSnapUp = 100.0f;

    /**
     * Max downward Z stick while walking. Farther drops use gravity instead of teleporting
     * to the valley floor.
     */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxFloorSnapDown = 100.0f;

    /**
     * Minimum surface Normal.Z to treat a blocked sweep as walkable terrain to slide along
     * rather than a wall to stop at. ~0.7 matches CMC's default ~44-degree walkable angle.
     */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float WalkableNormalZ = 0.7f;

    /**
     * Kept hovering this far above the floor surface (never touching it exactly). Without this,
     * the capsule starts every horizontal sweep already in contact with the ground, and UE
     * reports that as an immediate blocking hit — freezing movement even on flat terrain.
     */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float FloorClearance = 2.0f;

private:
    /**
     * Line-traces for WorldStatic under Location (capsule center).
     * On hit, OutCapsuleCenterZ is ImpactPoint.Z + scaled half-height.
     */
    bool TryFindFloorZ(const FVector& Location, float& OutCapsuleCenterZ) const;

    /** Spawn/activate helper: snaps Z to floor when a hit exists, otherwise leaves Location. */
    FVector SnapLocationToFloor(const FVector& Location) const;

    /**
     * Sweeps Delta; on a blocking hit against a walkable slope (see WalkableNormalZ), slides the
     * remainder along the surface instead of stopping dead, so real walls still block movement.
     * A near-vertical hit is climbed only by the measured step height, up to MaxFloorSnapUp.
     */
    void MoveHorizontal(const FVector& Delta);

    /** Sticks to nearby floor, or applies VerticalVelocity under gravity when unsupported. */
    void UpdateGroundedVertical(float DeltaTime);

    float VerticalVelocity = 0.0f;


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

    /** Authority-only: stop the shared attack timer. Override to add extra cleanup. */
    virtual void StopAttackOnServer();

    UFUNCTION(BlueprintImplementableEvent, Category = "Death")
    void OnDeathFeedback();

    /**
     * The authoritative GameMode when it owns monster lifetime, otherwise null. Resolved per call
     * instead of cached, since a pooled monster outlives nothing but is reused across waves.
     */
    INYMonsterLifecycleInterface* GetLifecycleHost() const;

    /** Seconds the corpse stays visible so every machine can play the death animation. */
    UPROPERTY(EditAnywhere, Category = "Death")
    float DeathDuration = 2.0f;

    /** Server-only. Clients infer the corpse state from replicated CurrentHp. */
    UPROPERTY(Transient)
    bool bIsDying = false;

private:
    FTimerHandle DeathTimerHandle;


// Attack
public:
    /** True while the current attack's freeze window is active (position holds; facing keeps tracking the target). */
    bool IsAttacking() const;

protected:
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackDamage = 10.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackRange = 100.0f;

    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackInterval = 0.5f;

    /** Presentation only. One is picked at random per attack. Must be designated in BP_MonsterMelee / BP_MonsterRanged. */
    UPROPERTY(EditDefaultsOnly, Category = "Attack")
    TArray<TObjectPtr<UAnimMontage>> AttackMontages;

    /** Freeze duration used when no montage is assigned for the chosen attack. */
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackFreezeDuration = 0.3f;

    /** Server-only. Gate for ProcessAttack: target, stagger, and range. Override to add extra conditions. */
    virtual bool CanAttack() const;

    /** Server-only. The actual attack effect (melee damage, projectile spawn, etc). */
    virtual void PerformAttack() {}

private:
    FTimerHandle AttackTimerHandle;

    /** Server-only. Bound to AttackTimerHandle; gates PerformAttack behind CanAttack. */
    void ProcessAttack();

    /**
     * NetMulticast, Reliable. Runs on every machine (including the server) so the local seek
     * simulation freezes in step; MontageToPlay may be null when no montage is assigned.
     */
    UFUNCTION(NetMulticast, Reliable, Category = "Attack")
    void Multicast_OnAttackStarted(UAnimMontage* MontageToPlay);

    float AttackEndTime = 0.0f;


// Multiplay
public:
    FORCEINLINE bool IsActive() const { return ActivationData.bIsActive; }

    /**
     * Authority-only: show at StartLocation and chase NewTarget.
     * A null NewTarget activates the monster idle — visible and collidable, but it never
     * seeks or attacks (training sandbox).
     */
    void ActivateOnServer(AActor* NewTarget, FVector StartLocation);

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
     * Fires on every machine that renders this monster, never on a dedicated server.
     * Lethal hits still fire this so the hit flash plays; stagger/knockback do not.
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
