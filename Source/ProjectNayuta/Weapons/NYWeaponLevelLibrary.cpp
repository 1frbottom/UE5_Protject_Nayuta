// Fill out your copyright notice in the Description page of Project Settings.

#include "Weapons/NYWeaponLevelLibrary.h"

#include "Engine/DataTable.h"

int32 NYWeaponLevel::GetMaxLevel(const UDataTable* WeaponLevelDataTable, FName WeaponID)
{
	if (!WeaponLevelDataTable || WeaponID.IsNone())
	{
		return 1;
	}

	int32 MaxLevel = 0;
	for (const TPair<FName, uint8*>& Pair : WeaponLevelDataTable->GetRowMap())
	{
		const FNYWeaponLevelRow* Row = reinterpret_cast<const FNYWeaponLevelRow*>(Pair.Value);
		if (Row && Row->WeaponID == WeaponID)
		{
			MaxLevel = FMath::Max(MaxLevel, Row->Level);
		}
	}

	return FMath::Max(1, MaxLevel);
}

bool NYWeaponLevel::TryGetRow(
	const UDataTable* WeaponLevelDataTable, FName WeaponID, int32 Level, FNYWeaponLevelRow& OutRow)
{
	if (!WeaponLevelDataTable || WeaponID.IsNone() || Level <= 0)
	{
		return false;
	}

	for (const TPair<FName, uint8*>& Pair : WeaponLevelDataTable->GetRowMap())
	{
		const FNYWeaponLevelRow* Row = reinterpret_cast<const FNYWeaponLevelRow*>(Pair.Value);
		if (Row && Row->WeaponID == WeaponID && Row->Level == Level)
		{
			OutRow = *Row;
			return true;
		}
	}

	return false;
}
