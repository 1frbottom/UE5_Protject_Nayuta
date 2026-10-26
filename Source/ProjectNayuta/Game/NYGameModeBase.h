// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NYGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	ANYGameModeBase();

	/** Server: clamp client-supplied player names so name tags cannot overflow. */
	virtual void ChangeName(AController* Controller, const FString& NewName, bool bNameChange) override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Multiplay")
	int32 MaxPlayerNameLength = 12;
};
