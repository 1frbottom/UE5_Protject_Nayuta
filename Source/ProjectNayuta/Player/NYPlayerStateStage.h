// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerStateBase.h"
#include "Data/NYRewardTypes.h"
#include "NYPlayerStateStage.generated.h"



UENUM(BlueprintType)
enum class ENYPlayerPhase : uint8
{
	Alive,
	Dead,
	Rewarding,
	Ready

};

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYPlayerStateStage : public ANYPlayerStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// Phase
public:
	FORCEINLINE ENYPlayerPhase GetPlayerPhase() const { return CurrPhase; }
	/** Alive or Ready (post-reward wait) — pawn accepts move/combat input. */
	FORCEINLINE bool CanControlPawn() const
	{
		return CurrPhase == ENYPlayerPhase::Alive || CurrPhase == ENYPlayerPhase::Ready;
	}
	void SetPlayerPhase(ENYPlayerPhase NewPhase);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrPhase)
	ENYPlayerPhase CurrPhase = ENYPlayerPhase::Alive;

	UFUNCTION()
	void OnRep_CurrPhase();


// Level
public:
	FORCEINLINE int32 GetCurrPlayerLv() const { return CurrPlayerLv; }
	FORCEINLINE int32 GetCurrExp() const { return CurrExp; }
	FORCEINLINE int32 GetMaxExp() const { return MaxExp; }
	FORCEINLINE int32 GetCurrGold() const { return CurrGold; }

	void AddExp(int32 InExp);
	void AddGold(int32 InGold);

	/** Server: reset run stats when a new stage attempt begins. */
	void ResetRunStats();

protected:
	void RefreshMaxExpForCurrentLevel();
	void NotifyLocalExpUI();
	void NotifyLocalGoldUI();
	void NotifyLocalLevelUpIfNeeded();

	/** Local-only: suppresses false OnPlayerLevelUp on first replicate / after ResetRunStats. */
	int32 LastNotifiedPlayerLevel = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stat")
	int32 MaxExp = 100;

	UPROPERTY(ReplicatedUsing = OnRep_CurrExp, BlueprintReadOnly, Category = "Stat")
	int32 CurrExp = 0;

	UFUNCTION()
	void OnRep_CurrExp();

	UPROPERTY(ReplicatedUsing = OnRep_CurrPlayerLv, BlueprintReadOnly, Category = "Stat")
	int32 CurrPlayerLv = 1;

	UFUNCTION()
	void OnRep_CurrPlayerLv();

	UPROPERTY(ReplicatedUsing = OnRep_CurrGold, BlueprintReadOnly, Category = "Stat")
	int32 CurrGold = 0;

	UFUNCTION()
	void OnRep_CurrGold();


// Hp
public:
	FORCEINLINE float GetMaxHp() const { return MaxHP; }
	FORCEINLINE float GetCurrHp() const { return CurrHp; }

	void SetCurrHp(float InHp);

	/** Server: set MaxHP and clamp CurrHp. */
	void SetMaxHp(float InMaxHp);

	//FORCEINLINE bool GetIsDead() const { return bIsDead; }
	//void SetbIsDead(bool InIsDead);

	void ApplyDamage(float InDamage);
	void AddMaxHp(float InAmount);

protected:
	UPROPERTY(Replicated, EditAnywhere, Category = "Stat")
	//float MaxHP = 100.0f;
	float MaxHP = 1000.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrHp, EditAnywhere, Category = "Stat")
	//float CurrHp = 100.0f;
	float CurrHp = 1000.0f;

	UFUNCTION()
	void OnRep_CurrHp();


// MoveSpeed
public:
	FORCEINLINE float GetMoveSpeed() const { return MoveSpeed; }

	void AddMoveSpeed(float InMoveSpeed);

	static constexpr float SprintSpeedBonus = 250.f;
	void SetSprinting(bool bSprint);
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

	/** MoveSpeed + Sprint Bonus Reflected to Pawn CMC */
	void ApplyMoveSpeedToPawn();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed, BlueprintReadOnly, Category = "Stat")
	float MoveSpeed = 250.0f;

	UFUNCTION()
	void OnRep_MoveSpeed();

	UPROPERTY(ReplicatedUsing = OnRep_bIsSprinting, BlueprintReadOnly, Category = "Stat")
	bool bIsSprinting = false;

	UFUNCTION()
	void OnRep_bIsSprinting();


// Reward
public:
	FORCEINLINE bool HasSelectedReward() const { return bHasSelectedReward; }
	FORCEINLINE const TArray<FNYRewardOffer>& GetPendingRewardOffers() const { return PendingRewardOffers; }

	/** Server: assign wave reward choices for the owning player. */
	void SetPendingRewardOffers(const TArray<FNYRewardOffer>& InOffers);

	/** Server: validate selection, apply reward, and move to Ready. */
	bool TrySelectReward(int32 SlotIndex);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_PendingRewardOffers, BlueprintReadOnly, Category = "Reward")
	TArray<FNYRewardOffer> PendingRewardOffers;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Reward")
	bool bHasSelectedReward = false;

	UFUNCTION()
	void OnRep_PendingRewardOffers();

	void ApplyReward(const FNYRewardOffer& Offer);
	void TryShowRewardUI();

};
