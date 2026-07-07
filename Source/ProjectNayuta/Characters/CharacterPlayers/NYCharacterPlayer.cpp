// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterPlayers/NYCharacterPlayer.h"

#include "ProjectNayuta.h"

#include "Engine/OverlapResult.h"
#include "Engine/LocalPlayer.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"

#include "Net/UnrealNetwork.h"      // DOREPLIFETIME

#include "Kismet/GameplayStatics.h"

#include "Game/NYGameModeStage.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "Player/NYPlayerControllerStage.h"
#include "Player/NYPlayerStateStage.h"

#include "Weapons/NYWeaponComponent.h"



ANYCharacterPlayer::ANYCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

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

        // test
    GetCharacterMovement()->JumpZVelocity = 1000.f;

    // Stat


    // Weapon
    DefaultWeaponComp = CreateDefaultSubobject<UNYWeaponComponent>(TEXT("DefaultWeaponComp"));

    // Multiplay
    bReplicates = true;     // make this actor replicated by network



}

void ANYCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ResolveMonsterSoftCollision();
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

    InitPlayerState();

    PC_ref = Cast<ANYPlayerControllerStage>(GetController());
}

void ANYCharacterPlayer::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);


}


// PlayerState
void ANYCharacterPlayer::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    InitPlayerState();
}

void ANYCharacterPlayer::InitPlayerState()
{
    PS_ref = GetPlayerState<ANYPlayerStateStage>();

    if (PS_ref)
    {
        PS_ref->ApplyMoveSpeedToPawn();
    }
}


// Input
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

        // Weapon swap
        EnhancedInputComponent->BindAction(WeaponSwapAction, ETriggerEvent::Started, this, &ANYCharacterPlayer::SwapWeaponSlots);
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
    if (!PS_ref || !PS_ref->CanControlPawn())
        return;

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // Calculate front and right vectors based on the direction the controller (camera) looks at
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // Move according to input value
        AddMovementInput(ForwardDirection, MovementVector.Y);
        AddMovementInput(RightDirection, MovementVector.X);
    }
}

void ANYCharacterPlayer::Sprint()
{
    if (!PS_ref || !PS_ref->CanControlPawn())
        return;

    if (HasAuthority())
    {
        PS_ref->SetSprinting(true);
    }
    else
    {
        Server_SetSprinting(true);
    }
}

void ANYCharacterPlayer::StopSprint()
{
    if (HasAuthority())
    {
        if (PS_ref)
            PS_ref->SetSprinting(false);
    }
    else
    {
        Server_SetSprinting(false);
    }
}

void ANYCharacterPlayer::Server_SetSprinting_Implementation(bool bSprint)
{
    if (ANYPlayerStateStage* PS = GetPlayerState<ANYPlayerStateStage>())
    {
        PS->SetSprinting(bSprint);
    }
}

void ANYCharacterPlayer::SwapWeaponSlots()
{
    if (!PS_ref || !PS_ref->CanControlPawn())
    {
        return;
    }

    if (HasAuthority())
    {
        if (DefaultWeaponComp)
        {
            DefaultWeaponComp->SwapWeaponSlots();
        }
    }
    else
    {
        Server_SwapWeaponSlots();
    }
}

void ANYCharacterPlayer::Server_SwapWeaponSlots_Implementation()
{
    if (ANYPlayerStateStage* PS = GetPlayerState<ANYPlayerStateStage>())
    {
        if (!PS->CanControlPawn())
        {
            return;
        }
    }

    if (DefaultWeaponComp)
    {
        DefaultWeaponComp->SwapWeaponSlots();
    }
}


// Stat
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
    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // stop dead animation?

}

void ANYCharacterPlayer::ResolveMonsterSoftCollision()
{
    if (!PS_ref)
    {
        PS_ref = GetPlayerState<ANYPlayerStateStage>();
    }

    if (!PS_ref || !PS_ref->CanControlPawn())
        return;

    const UCapsuleComponent* PlayerCapsule = GetCapsuleComponent();
    if (!PlayerCapsule || !PlayerCapsule->IsCollisionEnabled())
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    const float PlayerRadius = PlayerCapsule->GetScaledCapsuleRadius();
    const float QueryRadius = PlayerRadius + 50.0f + MonsterSeparationQueryPadding;

    TArray<FOverlapResult> OverlapResults;

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(MonsterSoftSeparation), false, this);

    FCollisionObjectQueryParams ObjectQueryParams;
    ObjectQueryParams.AddObjectTypesToQuery(ECC_MONSTER);

    World->OverlapMultiByObjectType(
        OverlapResults,
        GetActorLocation(),
        FQuat::Identity,
        ObjectQueryParams,
        FCollisionShape::MakeSphere(QueryRadius),
        QueryParams);

    if (OverlapResults.IsEmpty())
        return;

    const FVector PlayerLocation = GetActorLocation();
    const float PlayerPushWeight = GetLastMovementInputVector().SizeSquared2D() > KINDA_SMALL_NUMBER
        ? PlayerPushWeightWhileMoving
        : PlayerPushWeightWhileIdle;

    FVector AccumulatedPlayerOffset = FVector::ZeroVector;

    for (const FOverlapResult& Result : OverlapResults)
    {
        ANYMonsterBase* Monster = Cast<ANYMonsterBase>(Result.GetActor());
        if (!Monster || !Monster->IsActorTickEnabled())
            continue;

        const UCapsuleComponent* MonsterCapsule = Monster->GetCapsuleComponent();
        if (!MonsterCapsule)
            continue;

        FVector Separation = Monster->GetActorLocation() - PlayerLocation;
        Separation.Z = 0.0f;

        const float DistSq = Separation.SizeSquared2D();
        const float MinDist = PlayerRadius + MonsterCapsule->GetScaledCapsuleRadius();

        if (DistSq >= FMath::Square(MinDist))
            continue;

        FVector PushDir;
        float Penetration = MinDist;

        if (DistSq < KINDA_SMALL_NUMBER)
        {
            PushDir = FVector(FMath::FRandRange(-1.0f, 1.0f), FMath::FRandRange(-1.0f, 1.0f), 0.0f).GetSafeNormal();
        }
        else
        {
            const float Dist = FMath::Sqrt(DistSq);
            PushDir = Separation / Dist;
            Penetration = MinDist - Dist;
        }

        const float MonsterPushWeight = 1.0f - PlayerPushWeight;
        const FVector MonsterOffset = PushDir * Penetration * MonsterPushWeight;
        const FVector PlayerOffset = -PushDir * Penetration * PlayerPushWeight;

        Monster->AddActorWorldOffset(MonsterOffset, true);
        AccumulatedPlayerOffset += PlayerOffset;
    }

    if (!AccumulatedPlayerOffset.IsNearlyZero() && HasAuthority())
    {
        if (MaxPlayerSeparationPerTick > 0.0f)
        {
            AccumulatedPlayerOffset = AccumulatedPlayerOffset.GetClampedToMaxSize(MaxPlayerSeparationPerTick);
        }

        FHitResult Hit;
        GetCharacterMovement()->SafeMoveUpdatedComponent(
            AccumulatedPlayerOffset,
            GetActorRotation(),
            true,
            Hit);
    }
}


// Weapon
void ANYCharacterPlayer::ResetWeaponForNewRun()
{
    if (DefaultWeaponComp)
    {
        DefaultWeaponComp->ResetWeaponLevel();
    }
}
