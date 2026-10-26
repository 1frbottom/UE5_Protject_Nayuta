// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerBase.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerState.h"
#include "Game/NYGameInstance.h"



ANYPlayerControllerBase::ANYPlayerControllerBase()
{
    bReplicates = true;


}

void ANYPlayerControllerBase::BeginPlay()
{
    Super::BeginPlay();

     // Always equip the system common IMC (P key, etc.) on any level
    if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        if (IMC_System)
        {
            Subsystem->AddMappingContext(IMC_System, 1);
        }
    }

    // Local: PlayerState is recreated on non-seamless travel; seamless travel copies the name, so only resend when it differs.
    if (IsLocalPlayerController())
    {
        const UNYGameInstance* GI = GetGameInstance<UNYGameInstance>();
        if (GI && !GI->LocalPlayerName.IsEmpty() && (!PlayerState || PlayerState->GetPlayerName() != GI->LocalPlayerName))
        {
            SetName(GI->LocalPlayerName); // Client -> Server (ServerChangeName)
        }
    }
}

void ANYPlayerControllerBase::SetupInputComponent()
{
    Super::SetupInputComponent();

    // P key binding to call the common virtual function TogglePause
    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (PauseAction)
        {
            EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Started, this, &ANYPlayerControllerBase::TogglePause);
        }
    }
}


// Control
void ANYPlayerControllerBase::SetMouseSensitivity(float NewValue)
{
    MouseSensitivity = NewValue;
}


// Pause
void ANYPlayerControllerBase::TogglePause()
{
    if (!IsLocalPlayerController())
    {
        return;
    }

    // Menu levels have no pause state, so the owning widget decides what "back" means.
    OnPauseKeyPressed();
}
