// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NYWeaponComponent.generated.h"



class ANYAttackPlayerBase;
class UNYWeaponDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNYOnWeaponLevelChanged, int32, NewLevel);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class PROJECTNAYUTA_API UNYWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNYWeaponComponent();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	/** Server: swap weapon definition and restart the attack timer. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponDefinition(UNYWeaponDefinition* NewDefinition);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentWeaponLevel() const { return CurrentWeaponLevel; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMaxWeaponLevel() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanLevelUpWeapon() const;

	/** Server: increase weapon level and refresh combat stats. Returns false at max level. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool LevelUpWeapon();

	/** Server: reset to level 1 for a new stage run. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ResetWeaponLevel();

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNYOnWeaponLevelChanged OnWeaponLevelChanged;

protected:
	void ApplyWeaponDefinition();
	void RefreshAttackTimer();
	void NotifyWeaponLevelChanged();

	UFUNCTION()
	void OnRep_CurrentWeaponLevel();

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UNYWeaponDefinition> WeaponDefinition;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeaponLevel, BlueprintReadOnly, Category = "Weapon")
	int32 CurrentWeaponLevel = 1;

	TSubclassOf<ANYAttackPlayerBase> CurrentAttackClass;
	float CurrentDamage = 0.0f;
	float CurrentRange = 0.0f;
	float CurrentCooldown = 0.0f;

	FTimerHandle AttackTimer;

	void FireAttack();
};
