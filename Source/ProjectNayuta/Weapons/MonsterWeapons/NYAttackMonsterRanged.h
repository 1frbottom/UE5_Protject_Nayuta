// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/MonsterWeapons/NYAttackMonsterBase.h"
#include "NYAttackMonsterRanged.generated.h"

class USphereComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTNAYUTA_API ANYAttackMonsterRanged : public ANYAttackMonsterBase
{
	GENERATED_BODY()

public:
	ANYAttackMonsterRanged();

	/** Overrides the auto-computed straight velocity so callers can lob this along an arc. */
	void SetLaunchVelocity(const FVector& InVelocity);

protected:
	virtual void BeginPlay() override;


// Component
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;


// Attack
protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
