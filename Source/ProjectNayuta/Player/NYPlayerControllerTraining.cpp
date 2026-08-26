// Fill out your copyright notice in the Description page of Project Settings.

#include "Player/NYPlayerControllerTraining.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Blueprint/UserWidget.h"

#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Game/NYGameModeTraining.h"
#include "Player/NYPlayerStateStage.h"

ANYPlayerControllerTraining::ANYPlayerControllerTraining()
{
}

void ANYPlayerControllerTraining::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocalPlayerController())
	{
		CreateTrainingWidget();
		SetTrainingPanelVisible(true);
		ApplyTrainingInputMode();
	}
}

void ANYPlayerControllerTraining::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	if (IsLocalPlayerController())
	{
		ApplyTrainingInputMode();
	}
}

void ANYPlayerControllerTraining::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		if (ToggleTrainingPanelAction)
		{
			EnhancedInputComponent->BindAction(
				ToggleTrainingPanelAction,
				ETriggerEvent::Started,
				this,
				&ANYPlayerControllerTraining::ToggleTrainingPanel);
		}
	}
}

void ANYPlayerControllerTraining::ApplyTrainingInputMode()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	FlushPressedKeys();

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
		ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (IMC_InGame)
		{
			Subsystem->AddMappingContext(IMC_InGame, 0);
		}
	}

	if (bIsTrainingPanelOpen)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(InputMode);
		bShowMouseCursor = true;
	}
	else
	{
		SetInputMode(FInputModeGameOnly());
		bShowMouseCursor = false;
	}
}

void ANYPlayerControllerTraining::ToggleTrainingPanel()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	SetTrainingPanelVisible(!bIsTrainingPanelOpen);
	ApplyTrainingInputMode();
}

void ANYPlayerControllerTraining::CreateTrainingWidget()
{
	if (!IsLocalPlayerController() || !TrainingWidgetClass || TrainingWidgetRef)
	{
		return;
	}

	TrainingWidgetRef = CreateWidget<UUserWidget>(this, TrainingWidgetClass);
	if (TrainingWidgetRef)
	{
		TrainingWidgetRef->AddToViewport(10);
	}
}

void ANYPlayerControllerTraining::SetTrainingPanelVisible(bool bVisible)
{
	bIsTrainingPanelOpen = bVisible;

	if (!TrainingWidgetRef)
	{
		return;
	}

	TrainingWidgetRef->SetVisibility(
		bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void ANYPlayerControllerTraining::RequestSpawnTrainingMonster(TSubclassOf<ANYMonsterBase> MonsterClass)
{
	Server_SpawnTrainingMonster(MonsterClass);
}

void ANYPlayerControllerTraining::RequestSetTrainingMonsterMode(ENYTrainingMonsterMode Mode)
{
	Server_SetTrainingMonsterMode(Mode);
}

void ANYPlayerControllerTraining::RequestResetTrainingMonster()
{
	Server_ResetTrainingMonster();
}

float ANYPlayerControllerTraining::RequestSetPlayerMaxHp(float NewMaxHp)
{
	const float SanitizedMaxHp = FMath::Max(1.0f, FMath::RoundToFloat(NewMaxHp));
	Server_SetPlayerMaxHp(SanitizedMaxHp);
	return SanitizedMaxHp;
}

float ANYPlayerControllerTraining::RequestSetMonsterMaxHp(float NewMaxHp)
{
	const float SanitizedMaxHp = FMath::Max(1.0f, FMath::RoundToFloat(NewMaxHp));
	Server_SetMonsterMaxHp(SanitizedMaxHp);
	return SanitizedMaxHp;
}

float ANYPlayerControllerTraining::GetPlayerMaxHp() const
{
	if (const ANYPlayerStateStage* PS = GetPlayerState<ANYPlayerStateStage>())
	{
		return FMath::RoundToFloat(PS->GetMaxHp());
	}
	return 0.0f;
}

float ANYPlayerControllerTraining::GetPlayerCurrHp() const
{
	if (const ANYPlayerStateStage* PS = GetPlayerState<ANYPlayerStateStage>())
	{
		return FMath::RoundToFloat(PS->GetCurrHp());
	}
	return 0.0f;
}

float ANYPlayerControllerTraining::GetMonsterMaxHp() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const ANYGameModeTraining* GM = World->GetAuthGameMode<ANYGameModeTraining>())
		{
			return FMath::RoundToFloat(GM->GetTrainingMonsterMaxHp());
		}
	}

	if (SelectableMonsterClasses.Num() > 0)
	{
		return GetMonsterClassDefaultMaxHp(SelectableMonsterClasses[0]);
	}

	return 100.0f;
}

float ANYPlayerControllerTraining::GetMonsterCurrHp() const
{
	if (const UWorld* World = GetWorld())
	{
		if (const ANYGameModeTraining* GM = World->GetAuthGameMode<ANYGameModeTraining>())
		{
			return FMath::RoundToFloat(GM->GetTrainingMonsterCurrHp());
		}
	}

	return GetMonsterMaxHp();
}

float ANYPlayerControllerTraining::GetMonsterClassDefaultMaxHp(TSubclassOf<ANYMonsterBase> MonsterClass) const
{
	if (MonsterClass)
	{
		if (const ANYMonsterBase* CDO = MonsterClass->GetDefaultObject<ANYMonsterBase>())
		{
			return FMath::RoundToFloat(CDO->GetMaxHp());
		}
	}

	return 100.0f;
}

// Server
void ANYPlayerControllerTraining::Server_SpawnTrainingMonster_Implementation(TSubclassOf<ANYMonsterBase> MonsterClass)
{
	if (ANYGameModeTraining* GM = GetWorld()->GetAuthGameMode<ANYGameModeTraining>())
	{
		GM->SpawnTrainingMonster(MonsterClass, this);
	}
}

void ANYPlayerControllerTraining::Server_SetTrainingMonsterMode_Implementation(ENYTrainingMonsterMode Mode)
{
	if (ANYGameModeTraining* GM = GetWorld()->GetAuthGameMode<ANYGameModeTraining>())
	{
		GM->SetTrainingMonsterMode(Mode, this);
	}
}

void ANYPlayerControllerTraining::Server_ResetTrainingMonster_Implementation()
{
	if (ANYGameModeTraining* GM = GetWorld()->GetAuthGameMode<ANYGameModeTraining>())
	{
		GM->ResetTrainingMonster(this);
	}
}

void ANYPlayerControllerTraining::Server_SetPlayerMaxHp_Implementation(float NewMaxHp)
{
	if (ANYGameModeTraining* GM = GetWorld()->GetAuthGameMode<ANYGameModeTraining>())
	{
		GM->SetTrainingPlayerMaxHp(this, NewMaxHp);
	}
}

void ANYPlayerControllerTraining::Server_SetMonsterMaxHp_Implementation(float NewMaxHp)
{
	if (ANYGameModeTraining* GM = GetWorld()->GetAuthGameMode<ANYGameModeTraining>())
	{
		GM->SetTrainingMonsterMaxHp(NewMaxHp);
	}
}
