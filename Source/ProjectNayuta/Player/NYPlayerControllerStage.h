// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerControllerBase.h"
#include "NYPlayerControllerStage.generated.h"



/**
 * 
 */

class UInputAction;
class UInputMappingContext;

UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerStage : public ANYPlayerControllerBase
{
	GENERATED_BODY()
	
protected:
    virtual void BeginPlay() override;

// UI

    // Phase
public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HandleGamePhaseChanged(ENYGamePhase NewPhase);

    // Hp
public:
       // Event to call when the character's health changes, BP_PlayerController calls HUD to update
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdatePlayerHpUI(float HpPercentage);


    // Pause
public:
    virtual void TogglePause() override;

        // Event to call when the pause menu is toggled, BP_PlayerController calls HUD to update
    UFUNCTION(BlueprintImplementableEvent, Category = "UI") 
    void OnTogglePauseMenu();

protected:
    virtual void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_InGame;

    UPROPERTY(BlueprintReadWrite, Category = "UI")
    bool bIsPaused = false;


// Reward
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowRewardUI();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Server")
    void Server_SelectReward(int32 RewardID);


// Dead
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowDeadUI();

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowAliveUI();

// GameOver
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")

    void ShowGameOverUI();

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Server")
    void Server_RequestRetry();


// Multiplay
public:


protected:



};
