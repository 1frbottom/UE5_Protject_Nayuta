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


// Attack
protected:
	virtual void PerformAttack() override;

	/* Must be designated in BP_MonsterRanged */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	TSubclassOf<ANYAttackMonsterRanged> ProjectileClass;

	/** 0 ~ 1 : stiff to flat*/
	UPROPERTY(EditAnywhere, Category = "Attack")
	float ProjectileArcParam = 0.8f;

private:
	void FireProjectile();

};
