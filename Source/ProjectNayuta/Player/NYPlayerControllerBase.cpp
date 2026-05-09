// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerControllerBase.h"

#include "Blueprint/UserWidget.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"



ANYPlayerControllerBase::ANYPlayerControllerBase()
{

}

void ANYPlayerControllerBase::BeginPlay()
{
    Super::BeginPlay();

    // 어느 레벨이든 시스템 공통 IMC(P키 등)를 기본으로 장착
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

    // P키를 누르면 공통 가상 함수인 TogglePause가 실행되도록 바인딩
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
    // [공통 로직] 설정창이 열려있다면 닫고 참조를 초기화함
    if (SettingWidgetRef && SettingWidgetRef->IsInViewport())
    {
        SettingWidgetRef->RemoveFromParent();
        SettingWidgetRef = nullptr;
    }


}