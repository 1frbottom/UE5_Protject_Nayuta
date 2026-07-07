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

    if (IsLocalPlayerController())
    {
        ApplyInputConfig(ENYInputConfig::Gameplay);
    }
}

void ANYPlayerControllerStage::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    if (!IsLocalPlayerController())
    {
        return;
    }

    ENYInputConfig Config = ENYInputConfig::Gameplay;

    if (bIsPaused)
    {
        Config = ENYInputConfig::ModalUI;
    }
    else if (ANYGameStateStage* GS = GetWorld()->GetGameState<ANYGameStateStage>())
    {
        switch (GS->GetGamePhase())
        {
        case ENYGamePhase::Rewarding:
        case ENYGamePhase::GameOver:
        case ENYGamePhase::GameClear:
            Config = ENYInputConfig::ModalUI;
            break;
        default:
            break;
        }
    }

    ApplyInputConfig(Config);
}

void ANYPlayerControllerStage::SetupInputComponent()
{
    Super::SetupInputComponent();
}


// UI
void ANYPlayerControllerStage::ApplyInputConfig(ENYInputConfig Config)
{
    if (!IsLocalPlayerController())
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    switch (Config)
    {
    case ENYInputConfig::Gameplay:
        if (Subsystem && IMC_InGame)
        {
            Subsystem->AddMappingContext(IMC_InGame, 0);
        }

        SetInputMode(FInputModeGameOnly());
        bShowMouseCursor = false;
        bIsPaused = false;

        if (APawn* ControlledPawn = GetPawn())
        {
            ControlledPawn->EnableInput(this);
        }

        SetIgnoreMoveInput(false);
        SetIgnoreLookInput(false);
        break;

    case ENYInputConfig::ModalUI:
    {
        FModifyContextOptions ContextOptions;
        ContextOptions.bIgnoreAllPressedKeysUntilRelease = true;

        if (Subsystem && IMC_InGame)
        {
            Subsystem->RemoveMappingContext(IMC_InGame, ContextOptions);
        }

        FInputModeGameAndUI InputMode;
        InputMode.SetHideCursorDuringCapture(false);
        SetInputMode(InputMode);
        bShowMouseCursor = true;

        if (APawn* ControlledPawn = GetPawn())
        {
            ControlledPawn->DisableInput(this);
        }

        SetIgnoreMoveInput(true);
        SetIgnoreLookInput(true);
        break;
    }
    default:
        break;
    }
}

void ANYPlayerControllerStage::HandleGamePhaseChanged(ENYGamePhase NewPhase)
{
    if (!IsLocalController())
    {
        return;
    }

    switch (NewPhase)
    {
    case ENYGamePhase::Playing:
        ApplyInputConfig(ENYInputConfig::Gameplay);
        break;
    case ENYGamePhase::Rewarding:
        ApplyInputConfig(ENYInputConfig::ModalUI);
        break;
    case ENYGamePhase::GameOver:
        ApplyInputConfig(ENYInputConfig::ModalUI);
        ShowGameOverUI();
        break;
    case ENYGamePhase::GameClear:
    {
        int32 GoldEarned = 0;
        int32 WavesCleared = 0;

        if (ANYPlayerStateStage* PS = Cast<ANYPlayerStateStage>(PlayerState))
        {
            GoldEarned = PS->GetCurrGold();
        }

        if (const ANYGameStateStage* GS = GetWorld()->GetGameState<ANYGameStateStage>())
        {
            WavesCleared = GS->ReplicatedClearedWaveCount;
        }

        ApplyInputConfig(ENYInputConfig::ModalUI);
        ShowGameClearUI(WavesCleared, GoldEarned);
        break;
    }
    default:
        break;
    }
}

void ANYPlayerControllerStage::TogglePause()
{
    const bool bWasSettingOpen = (SettingWidgetRef != nullptr && SettingWidgetRef->IsInViewport());

    Super::TogglePause();

    if (bWasSettingOpen)
    {
        return;
    }

    if (bIsPaused)
    {
        OnTogglePauseMenu();
        ApplyInputConfig(ENYInputConfig::Gameplay);
    }
    else
    {
        OnTogglePauseMenu();
        ApplyInputConfig(ENYInputConfig::ModalUI);
        bIsPaused = true;
    }
}


// Reward
void ANYPlayerControllerStage::ConfirmRewardSelection(int32 SlotIndex)
{
    Server_SelectReward(SlotIndex);
}

void ANYPlayerControllerStage::Server_SelectReward_Implementation(int32 UpgradeIndex)
{
    ANYPlayerStateStage* PS = Cast<ANYPlayerStateStage>(PlayerState);
    if (!PS || PS->GetPlayerPhase() != ENYPlayerPhase::Rewarding)
    {
        return;
    }

    if (!PS->TrySelectReward(UpgradeIndex))
    {
        return;
    }

    if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnPlayerRewarded();
    }
}


// GameOver
void ANYPlayerControllerStage::Server_RequestRetry_Implementation()
{
    ANYPlayerStateStage* PS = Cast<ANYPlayerStateStage>(PlayerState);
    if (PS->GetPlayerPhase() != ENYPlayerPhase::Dead)
    {
        return;
    }

    if (ANYGameModeStage* GM = GetWorld()->GetAuthGameMode<ANYGameModeStage>())
    {
        GM->AddRetryVote();
    }
}

void ANYPlayerControllerStage::Server_RequestReturnToMainMenu_Implementation()
{
    if (ANYGameModeStage* GM = GetWorld()->GetAuthGameMode<ANYGameModeStage>())
    {
        GM->ReturnToMainMenu();
    }
}
