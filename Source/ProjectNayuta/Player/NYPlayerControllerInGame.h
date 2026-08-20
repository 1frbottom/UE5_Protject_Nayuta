// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerControllerBase.h"
#include "NYPlayerControllerInGame.generated.h"



/**
 * Stage and Training. Forwards HP to Blueprint HUD. Menu PC stays on Base.
 */
UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerInGame : public ANYPlayerControllerBase
{
	GENERATED_BODY()

// UI
public:
	UFUNCTION(BlueprintImplementableEvent, Category = "UI")
	void UpdatePlayerHpUI(float HpPercentage);

	/** Local: push PlayerState HP into UpdatePlayerHpUI. Call after the HUD widget exists. */
	UFUNCTION(BlueprintCallable, Category = "UI")
	void RefreshPlayerHpUI();
};
