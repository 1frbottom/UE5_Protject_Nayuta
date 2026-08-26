// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "NYHpBarWidgetMonster.generated.h"

class UProgressBar;

UCLASS()
class PROJECTNAYUTA_API UNYHpBarWidgetMonster : public UUserWidget
{
	GENERATED_BODY()


// UI
public:
	void UpdateHpBar(float InPercent);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HpProgressBar;

};
