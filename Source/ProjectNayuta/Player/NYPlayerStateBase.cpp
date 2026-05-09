// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerStateBase.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Game/NYGameMode.h"
#include "Player/NYPlayerControllerStage.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"



void ANYPlayerStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ANYPlayerStateBase, CurrState);

    DOREPLIFETIME(ANYPlayerStateBase, CurrExp);
    DOREPLIFETIME(ANYPlayerStateBase, MaxExp);
    DOREPLIFETIME(ANYPlayerStateBase, CurrPlayerLv);

    DOREPLIFETIME(ANYPlayerStateBase, CurrHp);
    DOREPLIFETIME(ANYPlayerStateBase, MaxHP);

    DOREPLIFETIME(ANYPlayerStateBase, MoveSpeed);


}

void ANYPlayerStateBase::SetPlayerState(ENYPlayerState NewState)
{
    if (!HasAuthority())
        return;

    CurrState = NewState;

    // 리슨서버용
    OnRep_CurrState();
}

void ANYPlayerStateBase::OnRep_CurrState()
{
    if (CurrState == ENYPlayerState::Dead)
    {
        if (ANYCharacterPlayer* Pwn_ref = Cast<ANYCharacterPlayer>(GetPawn()))
        {
            Pwn_ref->Die();
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
    else if (CurrState == ENYPlayerState::Rewarding)
    {

    }



}

void ANYPlayerStateBase::ApplyDamage(float DamageAmount)
{
    if (!HasAuthority() || !(CurrState == ENYPlayerState::Alive))
        return;

    CurrHp = FMath::Clamp(CurrHp - DamageAmount, 0.0f, MaxHP);
    
    // 서버용 수동 호출
    OnRep_CurrHp();

    // 캐릭터 사망
    if (CurrHp <= 0.0f)
    {
        CurrState = ENYPlayerState::Dead;
        OnRep_CurrState();

        // GameMode
        if (ANYGameMode* GM = GetWorld()->GetAuthGameMode<ANYGameMode>())
        {
            GM->OnPlayerDied(Cast<ANYPlayerControllerStage>(GetPlayerController()));
        }


    }
}

void ANYPlayerStateBase::SetCurrHp(float InHp)
{
    if (!HasAuthority())
        return;

    CurrHp = InHp;
    OnRep_CurrHp();
}

void ANYPlayerStateBase::OnRep_CurrHp()
{
    if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
    {
        float HpPercent = (MaxHP > 0.0f) ? (CurrHp / MaxHP) : 0.0f;
        PC->UpdatePlayerHpUI(HpPercent);

        // debug
        GEngine->AddOnScreenDebugMessage(
            -1, 5.f, FColor::Cyan,
            FString::Printf(TEXT("MaxHp : %f, CurrHp : %f"), MaxHP, CurrHp)
        );
    }
}

void ANYPlayerStateBase::AddExp(int32 InExp)
{
    if (!HasAuthority())
        return;

    CurrExp += InExp;

    while (CurrExp >= MaxExp)
    {
        CurrExp -= MaxExp;
        CurrPlayerLv++;
        // 다음 경험치통 DT에서 읽어오기

    }
}

void ANYPlayerStateBase::OnRep_CurrExp()
{
    // TODO : PC->HUD 통한 UpdateExpUI(CurrExp / MaxExp), PC의 UpdatePlayerHpUI처럼 하면댐



}

void ANYPlayerStateBase::AddMoveSpeed(float InMoveSpeed)
{
    if (!HasAuthority())
        return;

    MoveSpeed += InMoveSpeed;
    OnRep_MoveSpeed();
}

void ANYPlayerStateBase::OnRep_MoveSpeed()
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