// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerStage.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

#include "Game/NYGameModeStage.h"
#include "Game/NYGameStateStage.h"

#include "Player/NYPlayerStateStage.h"



void ANYPlayerControllerStage::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

}

void ANYPlayerControllerStage::HandleGamePhaseChanged(ENYGamePhase NewPhase)
{
    // only local
    if (!IsLocalController())
        return;
    switch (NewPhase)
    {
    case ENYGamePhase::Playing:
        // if needed : reward/gameover widget close, return to Playing UI
        break;
    case ENYGamePhase::Rewarding:
        ShowRewardUI();
        break;
    case ENYGamePhase::GameOver:
        ShowGameOverUI();
        break;
    default:
        break;
    }
}

void ANYPlayerControllerStage::SetupInputComponent()
{
    Super::SetupInputComponent();


}

void ANYPlayerControllerStage::TogglePause()
{
    // 1. Record the status of whether the setting window is open first
    bool bWasSettingOpen = (SettingWidgetRef != nullptr && SettingWidgetRef->IsInViewport());

    // 2. Execute the parent's setting window close logic
    Super::TogglePause();

    // 3. If the setting window is closed, exit the function here (only close the setting window with the P key)
    if (bWasSettingOpen)
    {
        return;
    }

    // 4. Execute the original pause logic only when the setting window is not open
    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    if (bIsPaused)
    {
        OnTogglePauseMenu();
        if (Subsystem && IMC_InGame) Subsystem->AddMappingContext(IMC_InGame, 0);
        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
        bIsPaused = false;
    }
    else
    {
        OnTogglePauseMenu();
        if (Subsystem && IMC_InGame) Subsystem->RemoveMappingContext(IMC_InGame);
        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);
        bShowMouseCursor = true;
        bIsPaused = true;
    }


}

void ANYPlayerControllerStage::Server_SelectReward_Implementation(int32 UpgradeIndex)
{
    ANYPlayerStateStage* PS = Cast<ANYPlayerStateStage>(PlayerState);
    if (PS->GetPlayerPhase() != ENYPlayerPhase::Rewarding)
        return;

    if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnPlayerRewarded();
    }
}

void ANYPlayerControllerStage::Server_RequestRetry_Implementation()
{
    ANYPlayerStateStage* PS = Cast<ANYPlayerStateStage>(PlayerState);
    if (PS->GetPlayerPhase() != ENYPlayerPhase::Dead)
        return;

    if (ANYGameModeStage* GM = GetWorld()->GetAuthGameMode<ANYGameModeStage>())
    {
        GM->AddRetryVote();
    }
}

