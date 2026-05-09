// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerMainmenu.h"



void ANYPlayerControllerMainmenu::BeginPlay()
{
    Super::BeginPlay();

    
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    bShowMouseCursor = true;


}