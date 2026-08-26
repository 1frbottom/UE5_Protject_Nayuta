// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NYAttackBase.generated.h"

class UStaticMeshComponent;

/**
Must be implemented { OnOverlapBegin() -> ApplyDamage() } in child class.
*/
UCLASS()
class PROJECTNAYUTA_API ANYAttackBase : public AActor
{
	GENERATED_BODY()

public:
	ANYAttackBase();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void InitAttackStat(float InDamage, float InRange);

// Component
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

// Attack
protected:
	/** Replicated so clients can scale VFX from BeginPlay (e.g. sword slash). */
	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Attack")
	float CurrentDamage = 0.0f;

	UPROPERTY(BlueprintReadOnly, Replicated, Category = "Attack")
	float CurrentRange = 0.0f;

};
