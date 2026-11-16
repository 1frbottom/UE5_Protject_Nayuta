// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "NYCharacterMovementComponent.generated.h"



/**
 * Player movement with sprint folded into CMC prediction.
 *
 * Sprint has to travel inside each saved move. Raising MaxWalkSpeed from a replicated property
 * lets the server and the owning client simulate at different speeds until the property arrives,
 * and every move in that window comes back as a correction.
 */
UCLASS()
class PROJECTNAYUTA_API UNYCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

	friend class FNYSavedMove_Character;

public:
	virtual float GetMaxSpeed() const override;

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

protected:
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;


// Sprint
public:
	/**
	 * Local: called from input on the locally controlled pawn. The server receives it through
	 * FLAG_Custom_0 on every move and replays it, so no RPC is needed.
	 */
	void SetWantsToSprint(bool bInWantsToSprint);

	FORCEINLINE bool WantsToSprint() const { return bWantsToSprint; }

protected:
	/** Added on top of MaxWalkSpeed (the PlayerState MoveSpeed stat) while sprinting. */
	UPROPERTY(EditDefaultsOnly, Category = "Sprint")
	float SprintSpeedBonus = 250.0f;

private:
	bool bWantsToSprint = false;

};
