// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Object Channels
#define ECC_PLAYER          ECC_GameTraceChannel1
#define ECC_MONSTER         ECC_GameTraceChannel2
#define ECC_PLAYERATTACK    ECC_GameTraceChannel3
#define ECC_MONSTERATTACK   ECC_GameTraceChannel4
#define ECC_ITEM            ECC_GameTraceChannel5
#define ECC_NPC				ECC_GameTraceChannel6

// Collision Profile
#define PROFILE_PLAYER			TEXT("NY_Player")
#define PROFILE_MONSTER			TEXT("NY_Monster")
#define PROFILE_PLAYER_ATTACK	TEXT("NY_PlayerAttack")
#define PROFILE_MONSTER_ATTACK	TEXT("NY_MonsterAttack")
#define PROFILE_ITEM			TEXT("NY_Item")