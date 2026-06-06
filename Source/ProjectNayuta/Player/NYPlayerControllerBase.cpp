// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerBase.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"



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

void ANYPlayerControllerBase::SetMouseSensitivity(float NewValue)
{
    MouseSensitivity = NewValue;
}

void ANYPlayerControllerBase::TogglePause()
{
    // [Common Logic] If the setting window is open, close it and initialize the reference
    if (SettingWidgetRef && SettingWidgetRef->IsInViewport())
    {
        SettingWidgetRef->RemoveFromParent();
        SettingWidgetRef = nullptr;
    }


}