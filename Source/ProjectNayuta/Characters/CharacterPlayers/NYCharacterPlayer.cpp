// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterPlayers/NYCharacterPlayer.h"

#include "ProjectNayuta.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

#include "Net/UnrealNetwork.h"      // DOREPLIFETIME

#include "Kismet/GameplayStatics.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"
#include "Engine/OverlapResult.h"

#include "Weapons/NYAttackBase.h"
#include "Weapons/NYWeaponComponent.h"

#include "Player/NYPlayerControllerStage.h"
#include "Player/NYPlayerStateBase.h"

#include "Game/NYGameMode.h"



ANYCharacterPlayer::ANYCharacterPlayer()
{
    // Camera
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->TargetArmLength = 800.0f;
    SpringArmComp->bUsePawnControlRotation = true;

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    CameraComp->bUsePawnControlRotation = false;

    // Collision
    GetCapsuleComponent()->SetCollisionProfileName(PROFILE_PLAYER);

    // Movement
    bUseControllerRotationYaw = false;
    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

    // Stat


    // Weapon
    DefaultWeaponComp = CreateDefaultSubobject<UNYWeaponComponent>(TEXT("DefaultWeaponComp"));

    // Multiplay
    bReplicates = true;     // make this actor replicated by network



}

void ANYCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();
	
    // core logic : should be started by server(host)
    if (HasAuthority())
    {
        


    }

}

// after possessed, server only
void ANYCharacterPlayer::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    InitPlayerState();

    PC_ref = Cast<ANYPlayerControllerStage>(GetController());

}

// after possessed, client only 
void ANYCharacterPlayer::PawnClientRestart()
{
    Super::PawnClientRestart();

    PC_ref = Cast<ANYPlayerControllerStage>(GetController());
    // add IMC into PlayerController
    if (PC_ref)
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC_ref->GetLocalPlayer()))
        {
            if (DefaultMappingContext)
            {
                Subsystem->AddMappingContext(DefaultMappingContext, 0);
            }
        }
    }

}

void ANYCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);


}

void ANYCharacterPlayer::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    InitPlayerState();
}

void ANYCharacterPlayer::InitPlayerState()
{
    PS_ref = GetPlayerState<ANYPlayerStateBase>();

    if (PS_ref)
    {
        GetCharacterMovement()->MaxWalkSpeed = PS_ref->GetMoveSpeed();

    }
}

// bind trigger to specific function 
// FInputActionValue : automatically processed when the player possess the pawn
void ANYCharacterPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);



    if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        // Look
        EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANYCharacterPlayer::Look);

        // Move
        EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANYCharacterPlayer::Move);

        // Sprint
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Started, this, &ANYCharacterPlayer::Sprint);
        EnhancedInputComponent->BindAction(SprintAction, ETriggerEvent::Completed, this, &ANYCharacterPlayer::StopSprint);

        // Jump
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
        EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);



    }


}

void ANYCharacterPlayer::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();

    if (PC_ref)
    {
        float Sensitivity = PC_ref->GetMouseSensitivity();

        AddControllerYawInput(LookAxisVector.X * Sensitivity);
        AddControllerPitchInput(LookAxisVector.Y * Sensitivity);
    }
}

void ANYCharacterPlayer::Move(const FInputActionValue& Value)
{
    if (!PS_ref && PS_ref->GetPlayerState() != ENYPlayerState::Alive)
        return;

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // 컨트롤러(카메라)가 바라보는 방향을 기준으로 전방 및 우측 벡터를 계산
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // 입력값에 따라 이동
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ANYCharacterPlayer::Sprint()
{
    GetCharacterMovement()->MaxWalkSpeed += 250.0f;

    if (!HasAuthority())
    {
        Server_Sprint();
    }
}
void ANYCharacterPlayer::Server_Sprint_Implementation()
{
    GetCharacterMovement()->MaxWalkSpeed += 250.0f;
}

void ANYCharacterPlayer::StopSprint()
{
    GetCharacterMovement()->MaxWalkSpeed -= 250.0f;

    if (!HasAuthority())
    {
        Server_StopSprint();
    }
}
void ANYCharacterPlayer::Server_StopSprint_Implementation()
{
    GetCharacterMovement()->MaxWalkSpeed -= 250.0f;
}

float ANYCharacterPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || !PS_ref)
        return 0.0f;

    PS_ref->ApplyDamage(DamageAmount);

    return DamageAmount;
}

void ANYCharacterPlayer::Die()
{
    // prevent falling down
    GetCharacterMovement()->DisableMovement();

    // Capule No Collision
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Body remains?

    // dead animation


}

void ANYCharacterPlayer::Revive()
{
    // Collision enable

    // Possess

}

