// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/NYGameStateBase.h"


#include "NYGameStateMainmenu.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerListChanged);

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYGameStateMainmenu : public ANYGameStateBase
{
	GENERATED_BODY()

public:

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


// Session Info
public:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplay")
	FName CurrentSessionName;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Multiplay")
	int32 MaxPlayers;

protected:


// PlayerState
public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Multiplay")
	TArray<class APlayerState*> GetSortedPlayerArray();

	UPROPERTY(BlueprintAssignable, Category = "Multiplay")
	FOnPlayerListChanged OnPlayerListChangedDelegate;
	
protected:
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;



};
