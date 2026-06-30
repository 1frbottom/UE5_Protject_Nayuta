// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameStateStage.h"

#include "Player/NYPlayerControllerStage.h"

#include "Net/UnrealNetwork.h"



void ANYGameStateStage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANYGameStateStage, CurrPhase);

	DOREPLIFETIME(ANYGameStateStage, ReplicatedKillCnt);
	DOREPLIFETIME(ANYGameStateStage, ReplicatedTargetKillCnt);


}


// Phase
void ANYGameStateStage::SetGamePhase(ENYGamePhase NewPhase)
{
	if (HasAuthority())
	{
		CurrPhase = NewPhase;

		OnRep_CurrPhase();
	}
}

void ANYGameStateStage::OnRep_CurrPhase()
{
	if (!GetWorld())
		return;

	// Get Local PC
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController())
			continue;

		if (ANYPlayerControllerStage* StagePC = Cast<ANYPlayerControllerStage>(PC))
		{
			StagePC->HandleGamePhaseChanged(CurrPhase);
		}
	}

	// change bgm

	// else?


}
