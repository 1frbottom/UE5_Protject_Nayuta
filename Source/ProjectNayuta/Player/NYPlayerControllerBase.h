// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NYPlayerControllerBase.generated.h"

/**
 * 
 */

class UUserWidget;

UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()
	
public:
    ANYPlayerControllerBase();

protected:
    virtual void BeginPlay() override;

// Control
public:
    FORCEINLINE float GetMouseSensitivity() const { return MouseSensitivity; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseSensitivity(float NewValue);

protected:
    virtual void SetupInputComponent() override;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<class UInputMappingContext> IMC_System;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float MouseSensitivity = 1.0f;

// Pause
public:
    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void TogglePause();

    // 현재 화면에 떠 있는 설정 위젯 참조
    UPROPERTY(BlueprintReadWrite, Category = "UI")
    TObjectPtr<UUserWidget> SettingWidgetRef;

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<class UInputAction> PauseAction;




};
