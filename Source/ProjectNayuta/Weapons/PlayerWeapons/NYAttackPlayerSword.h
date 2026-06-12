// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"
#include "NYAttackPlayerSword.generated.h"



class USphereComponent;

/** Melee attack — overlaps monsters around the instigator; no projectile movement. */
UCLASS()
class PROJECTNAYUTA_API ANYAttackPlayerSword : public ANYAttackPlayerBase
{
	GENERATED_BODY()

public:
	ANYAttackPlayerSword();

protected:
	virtual void BeginPlay() override;


protected:
	void ApplyMeleeDamageInRange();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> SphereComp;

	/** Brief lifetime for melee VFX. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack")
	float MeleeVisualDuration = 0.5f;

	/** Total forward sweep arc in degrees (e.g. 90 = 45 deg each side). 360 = full circle. */
	UPROPERTY(EditDefaultsOnly, Category = "Attack", meta = (ClampMin = "10.0", ClampMax = "360.0"))
	float MeleeSweepAngle = 60.0f;
};
