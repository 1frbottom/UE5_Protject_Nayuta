// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/MonsterWeapons/NYAttackMonsterBase.h"

#include "Components/ShapeComponent.h"
#include "ProjectNayuta.h"

void ANYAttackMonsterBase::BeginPlay()
{
	Super::BeginPlay();

	// BP에서 추가한 모든 충돌체를 찾아 몬스터 공격 채널로 강제 세팅
	TArray<UShapeComponent*> ShapeComps;
	GetComponents<UShapeComponent>(ShapeComps);

	for (UShapeComponent* Shape : ShapeComps)
	{
		Shape->SetCollisionProfileName(PROFILE_MONSTER_ATTACK);
	}


}