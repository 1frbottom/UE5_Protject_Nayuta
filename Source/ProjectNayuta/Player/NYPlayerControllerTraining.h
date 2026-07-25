// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerControllerBase.h"
#include "Game/NYGameModeTraining.h"
#include "NYPlayerControllerTraining.generated.h"

class ANYMonsterBase;
class UInputAction;
class UInputMappingContext;
class UUserWidget;

/**
 * Training-room PC: sandbox combat + training panel. Keeps Stage PC free of debug UI.
 */
UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerTraining : public ANYPlayerControllerBase
{
	GENERATED_BODY()

public:
	ANYPlayerControllerTraining();

protected:
	virtual void BeginPlay() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void SetupInputComponent() override;

// Input
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> IMC_InGame;

	/** Bind in editor (map key on IMC_System or IMC_InGame). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> ToggleTrainingPanelAction;

	/** Local: GameOnly when panel closed (mouse look); GameAndUI when open. */
	void ApplyTrainingInputMode();

	UFUNCTION(BlueprintCallable, Category = "Training")
	void ToggleTrainingPanel();

// Training
public:
	UFUNCTION(BlueprintCallable, Category = "Training")
	const TArray<TSubclassOf<ANYMonsterBase>>& GetSelectableMonsterClasses() const { return SelectableMonsterClasses; }

	UFUNCTION(BlueprintCallable, Category = "Training")
	void RequestSpawnTrainingMonster(TSubclassOf<ANYMonsterBase> MonsterClass);

	UFUNCTION(BlueprintCallable, Category = "Training")
	void RequestSetTrainingMonsterMode(ENYTrainingMonsterMode Mode);

	UFUNCTION(BlueprintCallable, Category = "Training")
	void RequestResetTrainingMonster();

	/** Sets MaxHP and refills CurrHp. Returns sanitized integer HP for UI text. */
	UFUNCTION(BlueprintCallable, Category = "Training")
	float RequestSetPlayerMaxHp(float NewMaxHp);

	/** Sets training monster MaxHp (applies now if spawned; kept for respawn/reset). Returns sanitized integer HP. */
	UFUNCTION(BlueprintCallable, Category = "Training")
	float RequestSetMonsterMaxHp(float NewMaxHp);

	UFUNCTION(BlueprintCallable, Category = "Training")
	float GetPlayerMaxHp() const;

	UFUNCTION(BlueprintCallable, Category = "Training")
	float GetPlayerCurrHp() const;

	UFUNCTION(BlueprintCallable, Category = "Training")
	float GetMonsterMaxHp() const;

	UFUNCTION(BlueprintCallable, Category = "Training")
	float GetMonsterCurrHp() const;

	/** Class Default MaxHp (combo 선택 직후 슬라이더 초기값용). */
	UFUNCTION(BlueprintCallable, Category = "Training")
	float GetMonsterClassDefaultMaxHp(TSubclassOf<ANYMonsterBase> MonsterClass) const;

protected:
	/** Classes shown in WBP combo / buttons. Fill on BP_PlayerController_Training. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Training")
	TArray<TSubclassOf<ANYMonsterBase>> SelectableMonsterClasses;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Training")
	TSubclassOf<UUserWidget> TrainingWidgetClass;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Training")
	TObjectPtr<UUserWidget> TrainingWidgetRef;

	UPROPERTY(BlueprintReadOnly, Category = "Training")
	bool bIsTrainingPanelOpen = true;

	/** Client → Server */
	UFUNCTION(Server, Reliable, Category = "Training")
	void Server_SpawnTrainingMonster(TSubclassOf<ANYMonsterBase> MonsterClass);

	/** Client → Server */
	UFUNCTION(Server, Reliable, Category = "Training")
	void Server_SetTrainingMonsterMode(ENYTrainingMonsterMode Mode);

	/** Client → Server */
	UFUNCTION(Server, Reliable, Category = "Training")
	void Server_ResetTrainingMonster();

	/** Client → Server */
	UFUNCTION(Server, Reliable, Category = "Training")
	void Server_SetPlayerMaxHp(float NewMaxHp);

	/** Client → Server */
	UFUNCTION(Server, Reliable, Category = "Training")
	void Server_SetMonsterMaxHp(float NewMaxHp);

	void CreateTrainingWidget();
	void SetTrainingPanelVisible(bool bVisible);
};
