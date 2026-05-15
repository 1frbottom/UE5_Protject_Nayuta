// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/NYPlayerControllerBase.h"
#include "NYPlayerControllerMainmenu.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTNAYUTA_API ANYPlayerControllerMainmenu : public ANYPlayerControllerBase
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;



// Multiplay
public:
	UFUNCTION(Server, Reliable, BlueprintCallable, Category = "Server")
	void Server_ToggleReady();

protected:




};
