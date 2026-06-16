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
    DOREPLIFETIME(ANYPlayerStateStage, CurrGold);

    DOREPLIFETIME(ANYPlayerStateStage, CurrHp);
    DOREPLIFETIME(ANYPlayerStateStage, MaxHP);

    DOREPLIFETIME(ANYPlayerStateStage, MoveSpeed);
    DOREPLIFETIME(ANYPlayerStateStage, bIsSprinting);

}

void ANYPlayerStateStage::SetPlayerPhase(ENYPlayerPhase NewPhase)
{
    if (!HasAuthority())
        return;

    CurrPhase = NewPhase;

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

    OnRep_CurrHp();

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

        // If it's my PC, update my Hpbar
        if (LocalPC->PlayerState == this)
        {
            LocalPC->UpdatePlayerHpUI(HpPercent);
        }
        else
        {


        }

        // debug
        //GEngine->AddOnScreenDebugMessage(
        //    -1, 5.f, FColor::Cyan,
        //    FString::Printf(TEXT("MaxHp : %f, CurrHp : %f"), MaxHP, CurrHp)
        //);
    }
}

void ANYPlayerStateStage::RefreshMaxExpForCurrentLevel()
{
    MaxExp = 0;

    if (const ANYGameModeStage* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ANYGameModeStage>() : nullptr)
    {
        MaxExp = GM->GetRequiredExpForLevel(CurrPlayerLv);
    }
}

void ANYPlayerStateStage::ResetRunStats()
{
    if (!HasAuthority())
    {
        return;
    }

    CurrPlayerLv = 1;
    CurrExp = 0;
    CurrGold = 0;
    RefreshMaxExpForCurrentLevel();

    LastNotifiedPlayerLevel = CurrPlayerLv;

    if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(GetPawn()))
    {
        Character->ResetWeaponForNewRun();
    }

    OnRep_CurrPlayerLv();
    OnRep_CurrExp();
    OnRep_CurrGold();
}

void ANYPlayerStateStage::AddExp(int32 InExp)
{
    if (!HasAuthority() || InExp <= 0)
    {
        return;
    }

    if (MaxExp <= 0)
    {
        RefreshMaxExpForCurrentLevel();
    }

    if (MaxExp <= 0)
    {
        return;
    }

    CurrExp += InExp;

    bool bLeveledUp = false;
    const ANYGameModeStage* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ANYGameModeStage>() : nullptr;

    while (MaxExp > 0 && CurrExp >= MaxExp)
    {
        const int32 NextLevel = CurrPlayerLv + 1;
        const int32 NextMaxExp = GM ? GM->GetRequiredExpForLevel(NextLevel) : 0;
        if (NextMaxExp <= 0)
        {
            CurrExp = MaxExp;
            break;
        }

        CurrExp -= MaxExp;
        CurrPlayerLv = NextLevel;
        MaxExp = NextMaxExp;
        bLeveledUp = true;
    }

    OnRep_CurrExp();

    if (bLeveledUp)
    {
        OnRep_CurrPlayerLv();
    }
}

void ANYPlayerStateStage::AddGold(int32 InGold)
{
    if (!HasAuthority() || InGold <= 0)
    {
        return;
    }

    CurrGold += InGold;
    OnRep_CurrGold();
}

void ANYPlayerStateStage::NotifyLocalExpUI()
{
    if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
    {
        if (PC->IsLocalPlayerController())
        {
            PC->UpdateExpUI(CurrExp, MaxExp, CurrPlayerLv);
        }
    }
}

void ANYPlayerStateStage::NotifyLocalGoldUI()
{
    if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
    {
        if (PC->IsLocalPlayerController())
        {
            PC->UpdateGoldUI(CurrGold);
        }
    }
}

void ANYPlayerStateStage::NotifyLocalLevelUpIfNeeded()
{
    if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
    {
        if (!PC->IsLocalPlayerController())
        {
            return;
        }

        if (LastNotifiedPlayerLevel > 0 && CurrPlayerLv > LastNotifiedPlayerLevel)
        {
            PC->OnPlayerLevelUp(CurrPlayerLv);
        }

        LastNotifiedPlayerLevel = CurrPlayerLv;
    }
}

void ANYPlayerStateStage::OnRep_CurrExp()
{


    NotifyLocalExpUI();
}

void ANYPlayerStateStage::OnRep_CurrPlayerLv()
{
    NotifyLocalExpUI();
    NotifyLocalLevelUpIfNeeded();
}

void ANYPlayerStateStage::OnRep_CurrGold()
{
    NotifyLocalGoldUI();
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
    ApplyMoveSpeedToPawn();
}

void ANYPlayerStateStage::SetSprinting(bool bSprint)
{
    if (!HasAuthority())
        return;
    if (CurrPhase != ENYPlayerPhase::Alive)
        return;
    if (bIsSprinting == bSprint)
        return;
    bIsSprinting = bSprint;
    OnRep_bIsSprinting(); // 리슨 서버 즉시 반영
}
void ANYPlayerStateStage::OnRep_bIsSprinting()
{
    ApplyMoveSpeedToPawn();
}

void ANYPlayerStateStage::ApplyMoveSpeedToPawn()
{
    APawn* Pawn_ref = GetPawn();
    if (!Pawn_ref)
        return;
    ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(Pawn_ref);
    if (!Character)
        return;
    float EffectiveSpeed = MoveSpeed;
    if (bIsSprinting && CurrPhase == ENYPlayerPhase::Alive)
    {
        EffectiveSpeed += SprintSpeedBonus;
    }
    Character->GetCharacterMovement()->MaxWalkSpeed = EffectiveSpeed;
}
