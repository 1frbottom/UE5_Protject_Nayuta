// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameModeBase.h"
#include "NYGameModeLobby.generated.h"

/**
 * Listen-server lobby map. Uses seamless travel for Lobby -> Stage while connected clients stay attached.
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameModeLobby : public ANYGameModeBase
{
	GENERATED_BODY()

public:
	ANYGameModeLobby();

protected:
	virtual void InitGameState() override;
};
