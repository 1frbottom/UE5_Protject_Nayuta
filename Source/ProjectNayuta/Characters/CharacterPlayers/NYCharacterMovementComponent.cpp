// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterPlayers/NYCharacterMovementComponent.h"

#include "GameFramework/Character.h"



class FNYSavedMove_Character : public FSavedMove_Character
{
public:
	typedef FSavedMove_Character Super;

	virtual void Clear() override
	{
		Super::Clear();
		bSavedWantsToSprint = false;
	}

	virtual uint8 GetCompressedFlags() const override
	{
		uint8 Result = Super::GetCompressedFlags();
		if (bSavedWantsToSprint)
		{
			Result |= FLAG_Custom_0;
		}
		return Result;
	}

	virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override
	{
		// Combining across a toggle would replay one of the two halves at the wrong speed.
		if (bSavedWantsToSprint != static_cast<const FNYSavedMove_Character*>(NewMove.Get())->bSavedWantsToSprint)
		{
			return false;
		}
		return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
	}

	virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override
	{
		Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

		if (const UNYCharacterMovementComponent* MoveComp = Cast<UNYCharacterMovementComponent>(C->GetCharacterMovement()))
		{
			bSavedWantsToSprint = MoveComp->bWantsToSprint;
		}
	}

	virtual void PrepMoveFor(ACharacter* C) override
	{
		Super::PrepMoveFor(C);

		if (UNYCharacterMovementComponent* MoveComp = Cast<UNYCharacterMovementComponent>(C->GetCharacterMovement()))
		{
			MoveComp->bWantsToSprint = bSavedWantsToSprint;
		}
	}

private:
	bool bSavedWantsToSprint = false;
};

class FNYNetworkPredictionData_Client_Character : public FNetworkPredictionData_Client_Character
{
public:
	typedef FNetworkPredictionData_Client_Character Super;

	explicit FNYNetworkPredictionData_Client_Character(const UCharacterMovementComponent& ClientMovement)
		: Super(ClientMovement)
	{
	}

	virtual FSavedMovePtr AllocateNewMove() override
	{
		return FSavedMovePtr(new FNYSavedMove_Character());
	}
};


float UNYCharacterMovementComponent::GetMaxSpeed() const
{
	const float BaseSpeed = Super::GetMaxSpeed();

	// Falling keeps the bonus so a jump mid-sprint carries its speed.
	if (bWantsToSprint && (IsMovingOnGround() || IsFalling()))
	{
		return BaseSpeed + SprintSpeedBonus;
	}

	return BaseSpeed;
}

FNetworkPredictionData_Client* UNYCharacterMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UNYCharacterMovementComponent* MutableThis = const_cast<UNYCharacterMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNYNetworkPredictionData_Client_Character(*this);
	}

	return ClientPredictionData;
}

void UNYCharacterMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWantsToSprint = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
}


// Sprint
void UNYCharacterMovementComponent::SetWantsToSprint(bool bInWantsToSprint)
{
	bWantsToSprint = bInWantsToSprint;
}
