// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameModeBase.h"
#include "NYGameModeTraining.generated.h"

class UNYWeaponDefinition;

/**
 * Standalone weapon / combat sandbox. Reuses Stage PC/PS/Pawn/GS without wave/reward loop.
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameModeTraining : public ANYGameModeBase
{
	GENERATED_BODY()

public:
	ANYGameModeTraining();

protected:
	virtual void BeginPlay() override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

// Training
protected:
	/** Granted to each player pawn on spawn when set. */
	UPROPERTY(EditDefaultsOnly, Category = "Training")
	TObjectPtr<UNYWeaponDefinition> DefaultWeaponDefinition;

	/** Server: Playing phase + Alive + optional default weapon. */
	void SetupTrainingPlayer(APlayerController* NewPlayer);
};
