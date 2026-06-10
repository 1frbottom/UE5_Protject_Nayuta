// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerStateBase.h"
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
	void SetPlayerPhase(ENYPlayerPhase NewPhase);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrPhase)
	ENYPlayerPhase CurrPhase = ENYPlayerPhase::Alive;

	UFUNCTION()
	void OnRep_CurrPhase();


	// Level / currency (persist for the stage run; reset on ServerTravel ?Restart)
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
	void NotifyLocalStatUI();

	// Hp
public:
	FORCEINLINE float GetMaxHp() const { return MaxHP; }

	FORCEINLINE float GetCurrHp() const { return CurrHp; }
	void SetCurrHp(float InHp);

	//FORCEINLINE bool GetIsDead() const { return bIsDead; }
	//void SetbIsDead(bool InIsDead);

	void ApplyDamage(float InDamage);

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stat")
	//float MaxHP = 100.0f;
	float MaxHP = 10000000.0f;


	UPROPERTY(ReplicatedUsing = OnRep_CurrHp, BlueprintReadOnly, Category = "Stat")
	//float CurrHp = 100.0f;
	float CurrHp = 10000000.0f;


	UFUNCTION()
	void OnRep_CurrHp();


	// MoveSpeed
public:
	FORCEINLINE float GetMoveSpeed() const { return MoveSpeed; }
	void AddMoveSpeed(float InMoveSpeed);

	static constexpr float SprintSpeedBonus = 250.f;
	void SetSprinting(bool bSprint);
	FORCEINLINE bool IsSprinting() const { return bIsSprinting; }

	// MoveSpeed + Sprint Bonus Reflected to Pawn CMC
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

	// Exp
protected:
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


	// WeaponComponent
public:

protected:
};
