// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameModeBase.h"

ANYGameModeBase::ANYGameModeBase()
{
	bUseSeamlessTravel = true;
}

// Server
void ANYGameModeBase::ChangeName(AController* Controller, const FString& NewName, bool bNameChange)
{
	Super::ChangeName(Controller, NewName.Left(MaxPlayerNameLength), bNameChange);
}

