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

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<USphereComponent> SphereComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComp;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);


};