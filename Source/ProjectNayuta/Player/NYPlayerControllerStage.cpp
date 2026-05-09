// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerStage.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

#include "Game/NYGameMode.h"
#include "Player/NYPlayerStateBase.h"



void ANYPlayerControllerStage::BeginPlay()
{
    Super::BeginPlay();

    FInputModeGameOnly InputMode;
    SetInputMode(InputMode);
    bShowMouseCursor = false;

}

void ANYPlayerControllerStage::SetupInputComponent()
{
    Super::SetupInputComponent();


}

void ANYPlayerControllerStage::TogglePause()
{
    // 1. 설정창이 열려있는지 상태를 먼저 기록
    bool bWasSettingOpen = (SettingWidgetRef != nullptr && SettingWidgetRef->IsInViewport());

    // 2. 부모의 설정창 닫기 로직 실행
    Super::TogglePause();

    // 3. 만약 설정창을 닫은 거라면 여기서 함수 종료 (P키로 설정창만 닫음)
    if (bWasSettingOpen)
    {
        return;
    }

    // 4. 설정창이 없을 때만 기존의 일시정지 로직 실행
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
    ANYPlayerStateBase* PS = Cast<ANYPlayerStateBase>(PlayerState);
    if (PS->GetPlayerPhase() != ENYPlayerPhase::Rewarding)
        return;

    if (ANYGameMode* GM = Cast<ANYGameMode>(GetWorld()->GetAuthGameMode()))
    {
        GM->OnPlayerRewarded();
    }
}

void ANYPlayerControllerStage::Server_RequestRetry_Implementation()
{
    ANYPlayerStateBase* PS = Cast<ANYPlayerStateBase>(PlayerState);
    if (PS->GetPlayerPhase() != ENYPlayerPhase::Dead)
        return;

    if (ANYGameMode* GM = GetWorld()->GetAuthGameMode<ANYGameMode>())
    {
        GM->AddRetryVote();
    }
}