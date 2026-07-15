// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "NYTrainingDummy.generated.h"

/**
 * Stationary damage target for LV_TrainingRoom. Never reports kills to GameMode.
 */
UCLASS()
class PROJECTNAYUTA_API ANYTrainingDummy : public ANYMonsterBase
{
	GENERATED_BODY()

public:
	ANYTrainingDummy();

protected:
	virtual void BeginPlay() override;
	virtual void OnRep_ActivationData() override;

// Stat
public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	/** Keep collision/visibility on so the dummy stays targetable when placed in-level. */
	void ForceActivePresentation();
};
