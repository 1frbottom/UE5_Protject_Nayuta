// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NYPlayerStateBase.generated.h"



UENUM(BlueprintType)
enum class ENYPlayerState : uint8
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
class PROJECTNAYUTA_API ANYPlayerStateBase : public APlayerState
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


	// State
public:
	FORCEINLINE ENYPlayerState GetPlayerState() { return CurrState; };
	void SetPlayerState(ENYPlayerState NewState);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrState)
	ENYPlayerState CurrState = ENYPlayerState::Alive;

	UFUNCTION()
	void OnRep_CurrState();


	// Level
public:

protected:




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
	float MaxHP = 100.0f;

	UPROPERTY(ReplicatedUsing = OnRep_CurrHp, BlueprintReadOnly, Category = "Stat")
	float CurrHp = 100.0f;

	UFUNCTION()
	void OnRep_CurrHp();


	// MoveSpeed
public:
	FORCEINLINE float GetMoveSpeed() const { return MoveSpeed; }
	void AddMoveSpeed(float InMoveSpeed);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MoveSpeed, BlueprintReadOnly, Category = "Stat")
	float MoveSpeed = 250.0f;

	UFUNCTION()
	void OnRep_MoveSpeed();

	// Exp
public:
	void AddExp(int32 InExp);


protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stat")
	int32 MaxExp;

	UPROPERTY(ReplicatedUsing = OnRep_CurrExp, BlueprintReadOnly, Category = "Stat")
	int32 CurrExp;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Stat")
	int32 CurrPlayerLv;

	UFUNCTION()
	void OnRep_CurrExp();


	// WeaponComponent
public:

protected:


};
