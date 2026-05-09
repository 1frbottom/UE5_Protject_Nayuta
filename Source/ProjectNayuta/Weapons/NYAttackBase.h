// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NYAttackBase.generated.h"

class UBoxComponent;
class UProjectileMovementComponent;

/**
Must be implemented { OnOverlapBegin() -> ApplyDamage() } in child class.
*/
UCLASS()
class PROJECTNAYUTA_API ANYAttackBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ANYAttackBase();

public:	
	virtual void InitAttackStat(float InDamage, float InRange);

protected:
	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	float CurrentDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Attack")
	float CurrentRange;




};
