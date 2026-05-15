// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerStateStage.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Game/NYGameModeStage.h"
#include "Player/NYPlayerControllerStage.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"



void ANYPlayerStateStage::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANYPlayerStateStage, CurrPhase);

    DOREPLIFETIME(ANYPlayerStateStage, CurrExp);
    DOREPLIFETIME(ANYPlayerStateStage, MaxExp);
    DOREPLIFETIME(ANYPlayerStateStage, CurrPlayerLv);

    DOREPLIFETIME(ANYPlayerStateStage, CurrHp);
    DOREPLIFETIME(ANYPlayerStateStage, MaxHP);

    DOREPLIFETIME(ANYPlayerStateStage, MoveSpeed);


}

void ANYPlayerStateStage::SetPlayerPhase(ENYPlayerPhase NewPhase)
{
    if (!HasAuthority())
        return;

    CurrPhase = NewPhase;

    // 리슨서버용
    OnRep_CurrPhase();
}

void ANYPlayerStateStage::OnRep_CurrPhase()
{
    if (CurrPhase == ENYPlayerPhase::Alive)
    {
        if (ANYCharacterPlayer* Character_ref = Cast<ANYCharacterPlayer>(GetPawn()))
        {
            Character_ref->Revive();
        }

        if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
        {
            if (PC->IsLocalPlayerController())
            {
                PC->ShowAliveUI();
            }
        }

    }
    else if (CurrPhase == ENYPlayerPhase::Dead)
    {
        if (ANYCharacterPlayer* Character_ref = Cast<ANYCharacterPlayer>(GetPawn()))
        {
            Character_ref->Die();
        }

        // WBP_Dead
        if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
        {
            if (PC->IsLocalPlayerController())
            {
                PC->ShowDeadUI();
            }
        }


    }
    else if (CurrPhase == ENYPlayerPhase::Rewarding)
    {

    }



}

void ANYPlayerStateStage::ApplyDamage(float DamageAmount)
{
    if (!HasAuthority() || !(CurrPhase == ENYPlayerPhase::Alive))
        return;

    CurrHp = FMath::Clamp(CurrHp - DamageAmount, 0.0f, MaxHP);

    // 서버용 수동 호출
    OnRep_CurrHp();

    // 캐릭터 사망
    if (CurrHp <= 0.0f)
    {
        SetPlayerPhase(ENYPlayerPhase::Dead);

        // GameMode
        if (ANYGameModeStage* GM = GetWorld()->GetAuthGameMode<ANYGameModeStage>())
        {
            GM->OnPlayerDied(Cast<ANYPlayerControllerStage>(GetPlayerController()));
        }


    }
}

void ANYPlayerStateStage::SetCurrHp(float InHp)
{
    if (!HasAuthority())
        return;

    CurrHp = InHp;
    OnRep_CurrHp();
}

void ANYPlayerStateStage::OnRep_CurrHp()
{
    // for party hpbar ui
    if (ANYPlayerControllerStage* LocalPC = Cast<ANYPlayerControllerStage>(GetWorld()->GetFirstPlayerController()))
    {
        float HpPercent = (MaxHP > 0.0f) ? (CurrHp / MaxHP) : 0.0f;

        // 내 PC면 내 Hpbar 업데이트
        if (LocalPC->PlayerState == this)
        {
            LocalPC->UpdatePlayerHpUI(HpPercent);
        }
        else
        {


        }

        // debug
        GEngine->AddOnScreenDebugMessage(
            -1, 5.f, FColor::Cyan,
            FString::Printf(TEXT("MaxHp : %f, CurrHp : %f"), MaxHP, CurrHp)
        );
    }
}

void ANYPlayerStateStage::AddExp(int32 InExp)
{
    if (!HasAuthority())
        return;

    // temp expception
    if (MaxExp <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("[ANYPlayerStateStage] MaxExp is 0 or less! Setting to default 100 to prevent infinite loop."));
        MaxExp = 100;
    }

    CurrExp += InExp;

    while (CurrExp >= MaxExp)
    {
        CurrExp -= MaxExp;
        CurrPlayerLv++;
        // 다음 경험치통 DT에서 읽어오기

    }

    OnRep_CurrExp();
}

void ANYPlayerStateStage::OnRep_CurrExp()
{
    // TODO : PC->HUD 통한 UpdateExpUI(CurrExp / MaxExp), PC의 UpdatePlayerHpUI처럼 하면댐



}



void ANYPlayerStateStage::AddMoveSpeed(float InMoveSpeed)
{
    if (!HasAuthority())
        return;

    MoveSpeed += InMoveSpeed;
    OnRep_MoveSpeed();
}

void ANYPlayerStateStage::OnRep_MoveSpeed()
{
    // 캐릭터 접근하여 실제이속 변경
    if (APawn* Pwn_ref = GetPawn())
    {
        if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(Pwn_ref))
        {
            Character->GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
        }
    }
}