// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/NYHpBarWidgetMonster.h"

#include "Components/ProgressBar.h"



void UNYHpBarWidgetMonster::UpdateHpBar(float InPercent)
{
	if (HpProgressBar)
	{
		HpProgressBar->SetPercent(InPercent);
	}
}