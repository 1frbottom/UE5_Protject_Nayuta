// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NYPlayerControllerBase.generated.h"

UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerBase : public APlayerController
{
	GENERATED_BODY()

public:
    ANYPlayerControllerBase();

protected:
    virtual void BeginPlay() override;
    virtual void SetupInputComponent() override;


// Control
public:
    FORCEINLINE float GetMouseSensitivity() const { return MouseSensitivity; }

    UFUNCTION(BlueprintCallable, Category = "Settings")
    void SetMouseSensitivity(float NewValue);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<class UInputMappingContext> IMC_System;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings", meta = (AllowPrivateAccess = "true"))
    float MouseSensitivity = 1.0f;


// Pause
public:
    /** Local: pause-key entry point. Base only backs out of menu widgets; subclasses override for real pausing. */
    UFUNCTION(BlueprintCallable, Category = "UI")
    virtual void TogglePause();

protected:
    /** Local: menu widget steps back one level (submenu -> root, root -> nothing). */
    UFUNCTION(BlueprintImplementableEvent, Category = "UI")
    void OnPauseKeyPressed();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
    TObjectPtr<class UInputAction> PauseAction;

};
