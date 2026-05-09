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
class ANYPlayerControllerStage;

UCLASS()
class PROJECTNAYUTA_API ANYCharacterPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	ANYCharacterPlayer();

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
	TObjectPtr<class ANYPlayerStateBase> PS_ref;

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
	void Server_Sprint();
	UFUNCTION(Server, Reliable)
	void Server_StopSprint();

	// Jump
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;


// Stat
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();

protected:
	void Revive();


// Weapon
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UNYWeaponComponent> DefaultWeaponComp;



// Multiplay
public:


};
