// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"
#include "NYAttackPlayerSword.generated.h"

class USphereComponent;

/** Melee attack — overlaps monsters around the instigator; held mesh stays on the character. */
UCLASS()
class PROJECTNAYUTA_API ANYAttackPlayerSword : public ANYAttackPlayerBase
{
	GENERATED_BODY()

public:
	ANYAttackPlayerSword();

protected:
	virtual void BeginPlay() override;


// Component
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> SphereComp;


// Attack
protected:
	void ApplyMeleeDamageInRange();

	/** Brief lifetime for slash VFX. Niagara belongs in BP (BeginPlay / Niagara component). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float MeleeVisualDuration = 0.5f;

	/** Sweep arc in degrees. 360 = full circle hit. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "10.0", ClampMax = "360.0"))
	float MeleeSweepAngle = 360.0f;

};
