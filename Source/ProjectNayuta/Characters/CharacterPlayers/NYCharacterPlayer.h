// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"

#include "NYCharacterPlayer.generated.h"



class USpringArmComponent;
class UCameraComponent;
class UInputComponent;
class UInputMappingContext;
class UInputAction;
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

public:
	virtual void PossessedBy(AController* NewController) override;

	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// PlayerController
	UPROPERTY(Transient)
	TObjectPtr<class ANYPlayerControllerStage> PC_ref;


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
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

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


// Stat
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();
	void Revive();

protected:
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



// Weapon
public:
	void ResetWeaponForNewRun();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UNYWeaponComponent> DefaultWeaponComp;



// Multiplay
public:


};
