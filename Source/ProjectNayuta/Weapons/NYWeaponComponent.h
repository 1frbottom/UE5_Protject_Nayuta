// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DataTable.h"

#include "NYWeaponComponent.generated.h"

class ANYAttackPlayerBase;

USTRUCT(BlueprintType)
struct FWeaponStatRow : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<ANYAttackPlayerBase> AttackClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float BaseDamage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float AttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Cooldown;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTNAYUTA_API UNYWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UNYWeaponComponent();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Data")
	TObjectPtr<UDataTable> WeaponDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Data")
	FName WeaponID;		// 예: "Weapon_Katana", "Weapon_Fireball"

	// 실제 인게임에서 사용할 현재 스탯
	TSubclassOf<ANYAttackPlayerBase> CurrentAttackClass;
	float CurrentDamage;
	float CurrentRange;
	float CurrentCooldown;

	FTimerHandle AttackTimer;

	void FireAttack();
		
};
