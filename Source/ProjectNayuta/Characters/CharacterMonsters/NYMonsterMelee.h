// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "NYMonsterMelee.generated.h"

class USphereComponent;

UCLASS()
class PROJECTNAYUTA_API ANYMonsterMelee : public ANYMonsterBase
{
	GENERATED_BODY()
	
public:
	ANYMonsterMelee();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	TObjectPtr<USphereComponent> AttackSphereComp;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackDamage = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Attack")
	float AttackInterval = 0.5f;

private:
	FTimerHandle AttackTimerHandle;

	UFUNCTION()
	void ProcessMeleeAttack();


};