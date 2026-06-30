// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYStageDataRows.h"

class UDataTable;

// WeaponLevel
namespace NYWeaponLevel
{
	int32 GetMaxLevel(const UDataTable* WeaponLevelDataTable, FName WeaponID);
	bool TryGetRow(const UDataTable* WeaponLevelDataTable, FName WeaponID, int32 Level, FNYWeaponLevelRow& OutRow);
}
