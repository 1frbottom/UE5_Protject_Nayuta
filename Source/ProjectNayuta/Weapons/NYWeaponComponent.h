// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NYWeaponComponent.generated.h"

class ANYAttackPlayerBase;
class UNYWeaponDefinition;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTNAYUTA_API UNYWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNYWeaponComponent();

	/** Server: swap weapon definition and restart the attack timer. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponDefinition(UNYWeaponDefinition* NewDefinition);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	void ApplyWeaponDefinition();
	void RefreshAttackTimer();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UNYWeaponDefinition> WeaponDefinition;

	TSubclassOf<ANYAttackPlayerBase> CurrentAttackClass;
	float CurrentDamage = 0.0f;
	float CurrentRange = 0.0f;
	float CurrentCooldown = 0.0f;

	FTimerHandle AttackTimer;

	void FireAttack();
};
