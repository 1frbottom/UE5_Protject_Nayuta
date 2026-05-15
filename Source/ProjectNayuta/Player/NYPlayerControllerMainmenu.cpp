// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerMainmenu.h"

#include "Player/NYPlayerStateMainmenu.h"



void ANYPlayerControllerMainmenu::BeginPlay()
{
    Super::BeginPlay();

    
    FInputModeGameAndUI InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputMode.SetHideCursorDuringCapture(false);
    SetInputMode(InputMode);
    bShowMouseCursor = true;


}

void ANYPlayerControllerMainmenu::Server_ToggleReady_Implementation()
{
    if (ANYPlayerStateMainmenu* PS = Cast<ANYPlayerStateMainmenu>(PlayerState))
    {
        PS->SetIsReadyLobby(!PS->GetIsReadyLobby());
    }
}
