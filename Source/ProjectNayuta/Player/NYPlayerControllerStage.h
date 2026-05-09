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

    // Hp
public:
        // 캐릭터의 체력이 변경될 때 호출할 이벤트, BP_PlayerController에서 HUD 호출해서 업데이트
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdatePlayerHpUI(float HpPercentage);


    // Pause
public:
    virtual void TogglePause() override;

        // BP에서 HUD 호출용
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
