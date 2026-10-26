// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerControllerInGame.h"
#include "Data/NYRewardTypes.h"
#include "NYPlayerControllerStage.generated.h"



class UInputAction;
class UInputMappingContext;

UENUM()
enum class ENYInputConfig : uint8
{
	Gameplay,
	ModalUI
};

/** Whether WBP_Pause is on screen. Settings is a submenu inside that widget, not a state here. */
UENUM(BlueprintType)
enum class ENYPauseMenuState : uint8
{
	Closed,
	Root
};

UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerStage : public ANYPlayerControllerInGame
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void SetupInputComponent() override;


// UI
public:
    // Phase
    UFUNCTION(BlueprintCallable, Category = "UI")
    void HandleGamePhaseChanged(ENYGamePhase NewPhase);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateExpUI(int32 CurrentExp, int32 MaxExpValue, int32 PlayerLevel);

    /** Client: fired once when CurrPlayerLv increases (FX, level-up UI). Not called on initial sync or stat reset. */
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnPlayerLevelUp(int32 NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void UpdateGoldUI(int32 GoldAmount);

    // Pause
    virtual void TogglePause() override;

    /** Local: moves the pause menu to NewState and applies the matching input config. */
    UFUNCTION(BlueprintCallable, Category = "UI")
    void SetPauseMenuState(ENYPauseMenuState NewState);

    UFUNCTION(BlueprintPure, Category = "UI")
    FORCEINLINE ENYPauseMenuState GetPauseMenuState() const { return PauseMenuState; }

    UFUNCTION(BlueprintPure, Category = "UI")
    FORCEINLINE bool IsPauseMenuOpen() const { return PauseMenuState != ENYPauseMenuState::Closed; }

    /** Local: shows or hides WBP_Pause. Submenu slides stay in the widget. */
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnPauseMenuStateChanged(ENYPauseMenuState NewState);

    /**
     * Local: if WBP_Pause has a submenu open, call its Set Submenu with that same value so it reverses.
     * Return true when the key was consumed. Unimplemented returns false and the pause menu closes.
     */
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    bool TryRevertPauseSubmenu();

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<UInputMappingContext> IMC_InGame;

    UPROPERTY(BlueprintReadOnly, Category = "UI")
    ENYPauseMenuState PauseMenuState = ENYPauseMenuState::Closed;

    /** Local: drops the pause menu without touching input config (the caller owns that). */
    void ResetPauseMenu();

    /** Client: gameplay IMC on/off, input mode, and cursor. */
    void ApplyInputConfig(ENYInputConfig Config);


// Reward
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowRewardUI(const TArray<FNYRewardOffer>& Offers);

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ConfirmRewardSelection(int32 SlotIndex);

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


// GameClear
public:
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void ShowGameClearUI(int32 WavesCleared, int32 GoldEarned);

    UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Server")
    void Server_RequestReturnToMainMenu();


// Multiplay
public:


protected:



};
