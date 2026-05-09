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
	
public:
	void UpdateHpBar(float InPercent);

protected:
	// UMG 블루프린트의 ProgressBar 이름과 반드시 동일해야 자동 바인딩
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HpProgressBar;


};
