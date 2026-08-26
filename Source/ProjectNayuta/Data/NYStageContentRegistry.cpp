// Fill out your copyright notice in the Description page of Project Settings.

#include "Data/NYStageContentRegistry.h"

#include "Monsters/NYMonsterDefinition.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"

const UNYMonsterDefinition* UNYStageContentRegistry::FindMonsterDefinition(FName MonsterType) const
{
	if (MonsterType.IsNone())
	{
		return nullptr;
	}

	for (const TObjectPtr<UNYMonsterDefinition>& Definition : MonsterDefinitions)
	{
		if (Definition && Definition->MonsterType == MonsterType)
		{
			return Definition;
		}
	}

	return nullptr;
}

TSubclassOf<ANYMonsterBase> UNYStageContentRegistry::ResolveMonsterClass(FName MonsterType) const
{
	const UNYMonsterDefinition* Definition = FindMonsterDefinition(MonsterType);
	return Definition ? Definition->MonsterClass : nullptr;
}
