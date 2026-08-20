// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/NYPlayerStateStage.h"

#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"

#include "Game/NYGameModeStage.h"
#include "Player/NYPlayerControllerInGame.h"
#include "Player/NYPlayerControllerStage.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Weapons/NYWeaponComponent.h"



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

    DOREPLIFETIME_CONDITION(ANYPlayerStateStage, PendingRewardOffers, COND_OwnerOnly);
    DOREPLIFETIME_CONDITION(ANYPlayerStateStage, bHasSelectedReward, COND_OwnerOnly);

}


// Phase
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
        TryShowRewardUI();
    }
    else if (CurrPhase == ENYPlayerPhase::Ready)
    {
        PendingRewardOffers.Empty();
    }



}


// Level
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

void ANYPlayerStateStage::RefreshMaxExpForCurrentLevel()
{
    MaxExp = 0;

    if (const ANYGameModeStage* GM = GetWorld() ? GetWorld()->GetAuthGameMode<ANYGameModeStage>() : nullptr)
    {
        MaxExp = GM->GetRequiredExpForLevel(CurrPlayerLv);
    }
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


// Hp
void ANYPlayerStateStage::SetCurrHp(float InHp)
{
    if (!HasAuthority())
        return;

    CurrHp = InHp;
    OnRep_CurrHp();
}

void ANYPlayerStateStage::SetMaxHp(float InMaxHp)
{
    if (!HasAuthority())
    {
        return;
    }

    MaxHP = FMath::Max(1.0f, InMaxHp);
    CurrHp = FMath::Clamp(CurrHp, 0.0f, MaxHP);
    OnRep_CurrHp();
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

void ANYPlayerStateStage::AddMaxHp(float InAmount)
{
    if (!HasAuthority() || InAmount <= 0.0f)
    {
        return;
    }

    MaxHP += InAmount;
    SetCurrHp(MaxHP);
}

void ANYPlayerStateStage::OnRep_CurrHp()
{
    ANYPlayerControllerInGame* OwnerPC = Cast<ANYPlayerControllerInGame>(GetPlayerController());
    if (!OwnerPC || !OwnerPC->IsLocalPlayerController())
    {
        return;
    }

    const float HpPercent = (MaxHP > 0.0f) ? (CurrHp / MaxHP) : 0.0f;
    OwnerPC->UpdatePlayerHpUI(HpPercent);
}


// MoveSpeed
void ANYPlayerStateStage::AddMoveSpeed(float InMoveSpeed)
{
    if (!HasAuthority())
        return;

    MoveSpeed += InMoveSpeed;
    OnRep_MoveSpeed();
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

void ANYPlayerStateStage::OnRep_MoveSpeed()
{
    ApplyMoveSpeedToPawn();
}

void ANYPlayerStateStage::OnRep_bIsSprinting()
{
    ApplyMoveSpeedToPawn();
}


// Reward
void ANYPlayerStateStage::SetPendingRewardOffers(const TArray<FNYRewardOffer>& InOffers)
{
    if (!HasAuthority())
    {
        return;
    }

    PendingRewardOffers = InOffers;
    bHasSelectedReward = false;
    OnRep_PendingRewardOffers();
}

bool ANYPlayerStateStage::TrySelectReward(int32 SlotIndex)
{
    if (!HasAuthority() || bHasSelectedReward || CurrPhase != ENYPlayerPhase::Rewarding)
    {
        return false;
    }

    if (!PendingRewardOffers.IsValidIndex(SlotIndex))
    {
        return false;
    }

    ApplyReward(PendingRewardOffers[SlotIndex]);

    bHasSelectedReward = true;
    PendingRewardOffers.Empty();
    SetPlayerPhase(ENYPlayerPhase::Ready);

    return true;
}

void ANYPlayerStateStage::OnRep_PendingRewardOffers()
{
    TryShowRewardUI();
}

void ANYPlayerStateStage::ApplyReward(const FNYRewardOffer& Offer)
{
    if (!HasAuthority())
    {
        return;
    }

    switch (Offer.RewardType)
    {
    case ENYRewardType::StatMaxHp:
        AddMaxHp(Offer.StatValue);
        break;

    case ENYRewardType::StatMoveSpeed:
        AddMoveSpeed(Offer.StatValue);
        break;

    case ENYRewardType::NewWeapon:
        if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(GetPawn()))
        {
            if (UNYWeaponComponent* WeaponComp = Character->GetWeaponComponent())
            {
                if (Offer.WeaponDefinition)
                {
                    WeaponComp->SetSecondaryWeaponDefinition(Offer.WeaponDefinition);
                }
            }
        }
        break;

    case ENYRewardType::WeaponUpgrade:
        if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(GetPawn()))
        {
            if (UNYWeaponComponent* WeaponComp = Character->GetWeaponComponent())
            {
                const bool bPrimary = Offer.WeaponSlot == ENYRewardWeaponSlot::Primary;
                WeaponComp->LevelUpSlot(bPrimary);
            }
        }
        break;

    default:
        break;
    }
}

void ANYPlayerStateStage::TryShowRewardUI()
{
    if (CurrPhase != ENYPlayerPhase::Rewarding || PendingRewardOffers.Num() <= 0)
    {
        return;
    }

    if (ANYPlayerControllerStage* PC = Cast<ANYPlayerControllerStage>(GetPlayerController()))
    {
        if (PC->IsLocalPlayerController())
        {
            PC->ShowRewardUI(PendingRewardOffers);
        }
    }
}
