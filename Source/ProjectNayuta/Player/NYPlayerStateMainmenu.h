// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerStateBase.h"
#include "NYPlayerStateMainmenu.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReadyStateChanged, bool, bIsReady);

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYPlayerStateMainmenu : public ANYPlayerStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void OnRep_PlayerId() override;


// Multiplay
public:
	FORCEINLINE bool GetIsReadyLobby() const { return bIsReadyLobby; }
	void SetIsReadyLobby(bool NewReady);

	UPROPERTY(BlueprintAssignable, Category = "Multiplay|Events")
	FOnReadyStateChanged OnReadyStateChangedDelegate;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_bIsReadyLobby, BlueprintReadOnly, Category = "Multiplay")
	bool bIsReadyLobby = false;

	UFUNCTION()
	void OnRep_bIsReadyLobby();

};
