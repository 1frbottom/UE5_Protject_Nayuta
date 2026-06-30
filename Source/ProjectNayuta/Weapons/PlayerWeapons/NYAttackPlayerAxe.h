// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"
#include "NYAttackPlayerAxe.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;

UCLASS()
class PROJECTNAYUTA_API ANYAttackPlayerAxe : public ANYAttackPlayerBase
{
	GENERATED_BODY()

public:
	ANYAttackPlayerAxe();

protected:
	virtual void BeginPlay() override;


// Component
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UBoxComponent> BoxComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;


// Attack
protected:
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

};
