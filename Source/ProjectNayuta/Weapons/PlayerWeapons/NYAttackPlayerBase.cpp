// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/PlayerWeapons/NYAttackPlayerBase.h"

#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"
#include "ProjectNayuta.h"
#include "Sound/SoundBase.h"

#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Weapons/NYWeaponComponent.h"
#include "Weapons/NYWeaponDefinition.h"

void ANYAttackPlayerBase::BeginPlay()
{
	Super::BeginPlay();

	// Find all collision components added in BP and force set them to the player attack channel
	TArray<UShapeComponent*> ShapeComps;
	GetComponents<UShapeComponent>(ShapeComps);

	for (UShapeComponent* Shape : ShapeComps)
	{
		Shape->SetCollisionProfileName(PROFILE_PLAYER_ATTACK);
	}

	TryHideHeldWeaponMesh();
	PlayAttackSound();
}

void ANYAttackPlayerBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	TryRestoreHeldWeaponMesh();

	Super::EndPlay(EndPlayReason);
}

void ANYAttackPlayerBase::TryHideHeldWeaponMesh()
{
	ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(GetInstigator());
	if (!Character)
	{
		return;
	}

	const UNYWeaponComponent* WeaponComp = Character->GetWeaponComponent();
	const UNYWeaponDefinition* Definition =
		WeaponComp ? WeaponComp->GetPrimarySlot().Definition.Get() : nullptr;

	if (!Definition || !Definition->bHideHeldMeshWhileAttacking)
	{
		return;
	}

	Character->PushHeldWeaponMeshHidden();
	bHidHeldWeaponMesh = true;
}

void ANYAttackPlayerBase::TryRestoreHeldWeaponMesh()
{
	if (!bHidHeldWeaponMesh)
	{
		return;
	}

	if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(GetInstigator()))
	{
		Character->PopHeldWeaponMeshHidden();
	}

	bHidHeldWeaponMesh = false;
}

void ANYAttackPlayerBase::PlayAttackSound()
{
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	const ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(GetInstigator());
	const UNYWeaponComponent* WeaponComp = Character ? Character->GetWeaponComponent() : nullptr;
	const UNYWeaponDefinition* Definition =
		WeaponComp ? WeaponComp->GetPrimarySlot().Definition.Get() : nullptr;
	if (!Definition || !Definition->AttackSound)
	{
		return;
	}

	UGameplayStatics::PlaySoundAtLocation(this, Definition->AttackSound, GetActorLocation());
}
