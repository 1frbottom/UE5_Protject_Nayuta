// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerMainmenu.h"

#include "Player/NYPlayerStateMainmenu.h"



void ANYPlayerControllerMainmenu::BeginPlay()
{
    Super::BeginPlay();

    FInputModeUIOnly InputMode;
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);
    bShowMouseCursor = true;


}


// Multiplay
void ANYPlayerControllerMainmenu::Server_ToggleReady_Implementation()
{
    if (ANYPlayerStateMainmenu* PS = Cast<ANYPlayerStateMainmenu>(PlayerState))
    {
        PS->SetIsReadyLobby(!PS->GetIsReadyLobby());
    }
}
