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

	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;


// Component
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> SphereComp;


// Attack
protected:
	void ApplyMeleeDamageInRange();
	void ApplyMeshSwingRotation(float Alpha);

	/** Brief lifetime for melee mesh swing. Slash Niagara belongs in BP (BeginPlay / Niagara component). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float MeleeVisualDuration = 0.5f;

	/** Sweep arc in degrees. 360 = full circle hit + full mesh spin. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack", meta = (ClampMin = "10.0", ClampMax = "360.0"))
	float MeleeSweepAngle = 360.0f;

	/** Extra yaw applied on top of mesh relative rotation while swinging (degrees). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attack")
	float MeshSwingYawOffset = 0.0f;

private:
	FRotator MeshBaseRelativeRotation = FRotator::ZeroRotator;
	float SwingElapsed = 0.0f;

};
