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


// Attack
protected:
	virtual void PerformAttack() override;

};
