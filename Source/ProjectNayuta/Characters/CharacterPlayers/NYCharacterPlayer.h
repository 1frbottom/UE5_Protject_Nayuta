// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "NYCharacterPlayer.generated.h"



class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputAction;
class UStaticMeshComponent;
class UAnimMontage;
class UNYWeaponComponent;

UCLASS()
class PROJECTNAYUTA_API ANYCharacterPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	ANYCharacterPlayer();

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void PossessedBy(AController* NewController) override;

	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// PlayerController
	UPROPERTY(Transient)
	TObjectPtr<class ANYPlayerControllerBase> PC_ref;


// PlayerState
private:
	UPROPERTY(Transient)
	TObjectPtr<class ANYPlayerStateStage> PS_ref;

	virtual void OnRep_PlayerState() override;

	void InitPlayerState();


// Camera
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArmComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComp;

// Input
public:
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

protected:
	// Look
	void Look(const FInputActionValue& Value);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	// Move
	void Move(const FInputActionValue& Value);
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;
	
	// Sprint
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> SprintAction;
	void Sprint();
	void StopSprint();

	UFUNCTION(Server, Reliable)
	void Server_SetSprinting(bool bSprint);

	// Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;

	// Weapon swap
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> WeaponSwapAction;
	void SwapWeaponSlots();

	UFUNCTION(Server, Reliable, Category = "Weapon")
	void Server_SwapWeaponSlots();



// Stat
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();
	void Revive();
	FORCEINLINE bool IsDead() const { return bIsDead; }

protected:
	/** Local presentation flag. Die/Revive already run on every machine via PlayerState phase. */
	UPROPERTY(Transient, BlueprintReadOnly, Category = "Stat")
	bool bIsDead = false;

	/** Push player and nearby monsters apart when capsules overlap (requires Player-Monster Overlap in collision presets). */
	void ResolveMonsterSoftCollision();

	/** Extra radius added to the overlap query beyond the player capsule. */
	UPROPERTY(EditDefaultsOnly, Category = "Collision|Separation")
	float MonsterSeparationQueryPadding = 10.0f;

	/** Player share of depenetration while moving (monster gets the rest). */
	UPROPERTY(EditDefaultsOnly, Category = "Collision|Separation")
	float PlayerPushWeightWhileMoving = 0.25f;

	/** Player share of depenetration while idle (monster swarm can push the player). */
	UPROPERTY(EditDefaultsOnly, Category = "Collision|Separation")
	float PlayerPushWeightWhileIdle = 0.45f;

	/** Cap total player displacement per frame to avoid spikes when many monsters overlap. */
	UPROPERTY(EditDefaultsOnly, Category = "Collision|Separation")
	float MaxPlayerSeparationPerTick = 8.0f;

// Hit
protected:
	/**
	 * Presentation only (montage, SFX, flash), authored in Character Blueprint.
	 * Fires on every machine that renders this pawn, never on a dedicated server.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Hit")
	void OnHitFeedback(float DamageTaken);

	/** Server: notify all machines a non-lethal hit landed. */
	UFUNCTION(NetMulticast, Unreliable, Category = "Hit")
	void Multicast_OnHitFeedback(float DamageTaken);

	/** Server: minimum seconds between hit-react multicasts. */
	UPROPERTY(EditDefaultsOnly, Category = "Hit")
	float HitReactRetriggerDelay = 0.35f;

private:
	float LastHitReactServerTime = -1.0f;

// Death
protected:
	/**
	 * Presentation only (montage, SFX, flash), authored in Character Blueprint.
	 * Fires on every machine that renders this pawn, never on a dedicated server.
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Death")
	void OnDeathFeedback();

// Weapon
public:
	FORCEINLINE UNYWeaponComponent* GetWeaponComponent() const { return DefaultWeaponComp; }

	void ResetWeaponForNewRun();

	/** Server: play the primary weapon attack montage on every machine that renders this pawn. */
	void PlayAttackMontage(UAnimMontage* MontageToPlay);

	/** Ref-counted hide for thrown attacks; safe if multiple projectiles overlap. */
	void PushHeldWeaponMeshHidden();
	void PopHeldWeaponMeshHidden();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UNYWeaponComponent> DefaultWeaponComp;

	/** Held weapon visual; mesh comes from PrimarySlot Definition.WeaponMesh. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UStaticMeshComponent> WeaponMeshComp;

	UFUNCTION()
	void UpdateWeaponVisual();

	void RefreshHeldWeaponMeshVisibility();

	int32 HeldWeaponMeshHideCount = 0;

	/**
	 * Presentation only (montage). Fires on every machine that renders this pawn,
	 * never on a dedicated server.
	 */
	UFUNCTION(NetMulticast, Unreliable, Category = "Attack")
	void Multicast_OnAttackStarted(UAnimMontage* MontageToPlay);



// Multiplay
public:


};
