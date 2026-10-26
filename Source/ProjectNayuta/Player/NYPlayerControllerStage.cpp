// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerStage.h"

#include "EnhancedInputSubsystems.h"
#include "GameFramework/PawnMovementComponent.h"

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

    if (IsPauseMenuOpen())
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

void ANYPlayerControllerStage::ApplyInputConfig(ENYInputConfig Config)
{
    if (!IsLocalPlayerController())
    {
        return;
    }

    // Clear held keys before IMC swap so Sprint/etc. get a clean Completed.
    FlushPressedKeys();

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    switch (Config)
    {
    case ENYInputConfig::Gameplay:
        if (Subsystem && IMC_InGame)
        {
            Subsystem->AddMappingContext(IMC_InGame, 0);
        }

        ApplyGameplayLookInput();
        break;

    case ENYInputConfig::ModalUI:
        if (Subsystem && IMC_InGame)
        {
            Subsystem->RemoveMappingContext(IMC_InGame);
        }

        {
            FInputModeGameAndUI InputMode;
            InputMode.SetHideCursorDuringCapture(false);
            SetInputMode(InputMode);
        }
        bShowMouseCursor = true;

        if (APawn* ControlledPawn = GetPawn())
        {
            if (UPawnMovementComponent* MoveComp = ControlledPawn->GetMovementComponent())
            {
                MoveComp->StopMovementImmediately();
            }
        }
        break;

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

    // Phase UI takes over the screen; drop the pause menu so it cannot linger underneath.
    ResetPauseMenu();

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
    if (!IsLocalPlayerController())
    {
        return;
    }

    // Reward / game-over / clear UI owns the screen, so the pause menu must not stack on top of it.
    if (ANYGameStateStage* GS = GetWorld()->GetGameState<ANYGameStateStage>())
    {
        if (GS->GetGamePhase() != ENYGamePhase::Playing)
        {
            return;
        }
    }

    if (PauseMenuState == ENYPauseMenuState::Closed)
    {
        SetPauseMenuState(ENYPauseMenuState::Root);
        return;
    }

    // Same call the settings button uses. Set Submenu reverses when the value matches CurrentSubMenu.
    if (TryRevertPauseSubmenu())
    {
        return;
    }

    SetPauseMenuState(ENYPauseMenuState::Closed);
}

void ANYPlayerControllerStage::SetPauseMenuState(ENYPauseMenuState NewState)
{
    if (!IsLocalPlayerController() || PauseMenuState == NewState)
    {
        return;
    }

    PauseMenuState = NewState;
    OnPauseMenuStateChanged(NewState);

    ApplyInputConfig(NewState == ENYPauseMenuState::Closed ? ENYInputConfig::Gameplay : ENYInputConfig::ModalUI);
}

void ANYPlayerControllerStage::ResetPauseMenu()
{
    if (PauseMenuState == ENYPauseMenuState::Closed)
    {
        return;
    }

    PauseMenuState = ENYPauseMenuState::Closed;
    OnPauseMenuStateChanged(ENYPauseMenuState::Closed);
}

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
