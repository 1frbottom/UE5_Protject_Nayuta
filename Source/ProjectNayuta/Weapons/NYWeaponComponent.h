// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "NYWeaponComponent.generated.h"

class ANYAttackPlayerBase;
class UNYWeaponDefinition;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNYOnWeaponLevelChanged, int32, NewLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FNYOnWeaponSlotsChanged);

USTRUCT(BlueprintType)
struct FNYWeaponSlot
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	TObjectPtr<UNYWeaponDefinition> Definition = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	int32 Level = 1;
};

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


// Weapon
public:
	/** Server: set the primary weapon definition and reset its level to 1. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetWeaponDefinition(UNYWeaponDefinition* NewDefinition);

	/** Server: set the secondary weapon definition and reset its level to 1. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SetSecondaryWeaponDefinition(UNYWeaponDefinition* NewDefinition);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanSwapWeaponSlots() const;

	/** Server: swap primary and secondary slot contents. No-op when secondary is empty. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void SwapWeaponSlots();

	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FNYWeaponSlot& GetPrimarySlot() const { return PrimarySlot; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	const FNYWeaponSlot& GetSecondarySlot() const { return SecondarySlot; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetCurrentWeaponLevel() const { return PrimarySlot.Level; }

	UFUNCTION(BlueprintPure, Category = "Weapon")
	int32 GetMaxWeaponLevel() const;

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanLevelUpWeapon() const;

	/** Server: increase weapon level for the given slot. Returns false at max level or empty slot. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool LevelUpSlot(bool bPrimary);

	UFUNCTION(BlueprintPure, Category = "Weapon")
	bool CanLevelUpSlot(bool bPrimary) const;

	/** Server: increase primary weapon level. Returns false at max level. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	bool LevelUpWeapon();

	/** Server: reset both slots for a new stage run. */
	UFUNCTION(BlueprintCallable, Category = "Weapon")
	void ResetWeaponLevel();

	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNYOnWeaponLevelChanged OnWeaponLevelChanged;

	// TODO : use for UI, VFX
	UPROPERTY(BlueprintAssignable, Category = "Weapon")
	FNYOnWeaponSlotsChanged OnWeaponSlotsChanged;

protected:
	void ApplyWeaponDefinition();
	void RefreshAttackTimer();
	void NotifyWeaponLevelChanged();
	void NotifyWeaponSlotsChanged();

	int32 GetMaxWeaponLevelForSlot(const FNYWeaponSlot& Slot) const;
	const FNYWeaponSlot& GetSlot(bool bPrimary) const;

	UFUNCTION()
	void OnRep_WeaponSlots();

	UPROPERTY(EditDefaultsOnly, ReplicatedUsing = OnRep_WeaponSlots, Category = "Weapon")
	FNYWeaponSlot PrimarySlot;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponSlots, BlueprintReadOnly, Category = "Weapon")
	FNYWeaponSlot SecondarySlot;

	TSubclassOf<ANYAttackPlayerBase> CurrentAttackClass;
	float CurrentDamage = 0.0f;
	float CurrentRange = 0.0f;
	float CurrentCooldown = 0.0f;

	FTimerHandle AttackTimer;

	void FireAttack();

};
