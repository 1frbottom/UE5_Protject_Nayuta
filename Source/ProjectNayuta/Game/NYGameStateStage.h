// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameStateBase.h"
#include "Engine/DataTable.h"

#include "NYGameStateStage.generated.h"



UENUM(BlueprintType)
enum class ENYGamePhase : uint8
{
	Waiting,
	Playing,
	Rewarding,
	GameOver

};

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameStateStage : public ANYGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// Phase
public:
	FORCEINLINE ENYGamePhase GetGamePhase() { return CurrPhase; }
	void SetGamePhase(ENYGamePhase NewPhase);

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrPhase)
	ENYGamePhase CurrPhase;

	UFUNCTION()
	void OnRep_CurrPhase();


// Info
public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Info")
	int32 ReplicatedKillCnt = 0;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Info")
	int32 ReplicatedTargetKillCnt = 0;

	/** Shared stage config — readable on server and clients (not replicated; set per map). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Data")
	TObjectPtr<UDataTable> WeaponLevelDataTable;

};
