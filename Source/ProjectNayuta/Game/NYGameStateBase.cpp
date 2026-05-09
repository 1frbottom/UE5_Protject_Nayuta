// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYGameStateBase.h"

#include "Player/NYPlayerControllerStage.h"

#include "Net/UnrealNetwork.h"



void ANYGameStateBase::OnRep_CurrPhase()
{
	// Get Local PC
	ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		if (CurrPhase == ENYGamePhase::Playing)
		{
			// 필요시 웨이브 시작 UI 팝업
		}
		else if (CurrPhase == ENYGamePhase::Rewarding)
		{
			PC->ShowRewardUI();

		}
		else if (CurrPhase == ENYGamePhase::GameOver)
		{
			PC->ShowGameOverUI();

		}
	}

	// change bgm

	// else?


}

void ANYGameStateBase::SetGamePhase(ENYGamePhase NewPhase)
{
    if (HasAuthority())
    {
        CurrPhase = NewPhase;

        OnRep_CurrPhase(); // 서버는 OnRep이 자동 호출되지 않으므로 수동 호출
    }
}

void ANYGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ANYGameStateBase, CurrPhase);

	DOREPLIFETIME(ANYGameStateBase, ReplicatedKillCnt);
	DOREPLIFETIME(ANYGameStateBase, ReplicatedTargetKillCnt);


}

