// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/NYAttackBase.h"
#include "NYAttackMonsterBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYAttackMonsterBase : public ANYAttackBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;


	
};
