// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "NYMonsterRanged.generated.h"



class ANYAttackMonsterRanged;

UCLASS()
class PROJECTNAYUTA_API ANYMonsterRanged : public ANYMonsterBase
{
	GENERATED_BODY()
	
public:
	ANYMonsterRanged();

protected:
	virtual void BeginPlay() override;

protected:
	/* Must be designated in BP_MonsterRanged */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TSubclassOf<ANYAttackMonsterRanged> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackRange = 500.0f;

	UPROPERTY(VisibleAnywhere, Category = "Attack")
	float AttackRangeSqrd;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float FireRate = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 15.0f;

private:
	FTimerHandle FireCheckTimerHandle;
	float LastFireTime;

	UFUNCTION()
	void CheckAndFire();

	void FireProjectile();


};