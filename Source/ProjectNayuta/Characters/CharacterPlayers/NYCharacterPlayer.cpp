// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/CharacterPlayers/NYCharacterPlayer.h"

#include "ProjectNayuta.h"

#include "Engine/OverlapResult.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"

#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"

#include "Net/UnrealNetwork.h"      // DOREPLIFETIME

#include "Kismet/GameplayStatics.h"

#include "Game/NYGameModeStage.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"

#include "Player/NYPlayerControllerBase.h"
#include "Player/NYPlayerStateStage.h"

#include "Weapons/NYWeaponComponent.h"
#include "Weapons/NYWeaponDefinition.h"

#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"



ANYCharacterPlayer::ANYCharacterPlayer()
{
    PrimaryActorTick.bCanEverTick = true;

    // Camera
    SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
    SpringArmComp->SetupAttachment(RootComponent);
    SpringArmComp->TargetArmLength = 800.0f;
    SpringArmComp->bUsePawnControlRotation = true;
    SpringArmComp->bInheritPitch = true;
    SpringArmComp->bInheritYaw = true;
    SpringArmComp->bInheritRoll = true;
    SpringArmComp->SetRelativeRotation(FRotator::ZeroRotator);

    CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
    CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName);
    CameraComp->bUsePawnControlRotation = false;

    // Collision
    GetCapsuleComponent()->SetCollisionProfileName(PROFILE_PLAYER);

    // Movement
    // Face camera yaw so the spring arm sits on the character's back.
    bUseControllerRotationYaw = true;
    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->bUseControllerDesiredRotation = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);

        // test
    GetCharacterMovement()->JumpZVelocity = 700.f;

    // Stat


    // Weapon
    DefaultWeaponComp = CreateDefaultSubobject<UNYWeaponComponent>(TEXT("DefaultWeaponComp"));

    WeaponMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMeshComp"));
    WeaponMeshComp->SetupAttachment(GetMesh());
    WeaponMeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    // Multiplay
    bReplicates = true;     // make this actor replicated by network



}

void ANYCharacterPlayer::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    ResolveMonsterSoftCollision();
    UpdateLocalAim(DeltaTime);
}

void ANYCharacterPlayer::BeginPlay()
{
	Super::BeginPlay();

	if (DefaultWeaponComp)
	{
		DefaultWeaponComp->OnWeaponSlotsChanged.AddDynamic(this, &ANYCharacterPlayer::UpdateWeaponVisual);
		UpdateWeaponVisual();
	}
}

void ANYCharacterPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (DefaultWeaponComp)
	{
		DefaultWeaponComp->OnWeaponSlotsChanged.RemoveDynamic(this, &ANYCharacterPlayer::UpdateWeaponVisual);
	}

	Super::EndPlay(EndPlayReason);
}

// after possessed, server only
void ANYCharacterPlayer::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);

    InitPlayerState();

    PC_ref = Cast<ANYPlayerControllerBase>(GetController());

    EnsureDefaultCameraPitch();

}

// after possessed, client only 
void ANYCharacterPlayer::PawnClientRestart()
{
    Super::PawnClientRestart();

    InitPlayerState();

    PC_ref = Cast<ANYPlayerControllerBase>(GetController());

    EnsureDefaultCameraPitch();
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

ANYPlayerStateStage* ANYCharacterPlayer::ResolvePlayerState()
{
    if (!PS_ref)
    {
        InitPlayerState();
    }

    return PS_ref;
}

void ANYCharacterPlayer::EnsureDefaultCameraPitch()
{
    // Local: control rotation belongs to the owning client; the server copy would be overwritten anyway.
    if (!IsLocallyControlled())
    {
        return;
    }

    AController* PossessingController = GetController();
    if (!PossessingController)
    {
        return;
    }

    FRotator ControlRot = PossessingController->GetControlRotation();
    if (FMath::Abs(ControlRot.Pitch) < 1.0f)
    {
        ControlRot.Pitch = DefaultCameraPitch;
        PossessingController->SetControlRotation(ControlRot);
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

        // Attack. Canceled too, or an early release under a Hold-style trigger leaves the server firing.
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ANYCharacterPlayer::StartAttack);
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Completed, this, &ANYCharacterPlayer::StopAttack);
        EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Canceled, this, &ANYCharacterPlayer::StopAttack);
    }


}

void ANYCharacterPlayer::Look(const FInputActionValue& Value)
{
    // Local: camera stays free while dead or in reward UI; only move/fire are phase-gated.
    if (!PC_ref)
    {
        PC_ref = Cast<ANYPlayerControllerBase>(GetController());
    }

    if (!PC_ref)
    {
        return;
    }

    const FVector2D LookAxisVector = Value.Get<FVector2D>();
    const float Sensitivity = PC_ref->GetMouseSensitivity();
    AddControllerYawInput(LookAxisVector.X * Sensitivity);
    AddControllerPitchInput(LookAxisVector.Y * Sensitivity);
}

void ANYCharacterPlayer::Move(const FInputActionValue& Value)
{
    const ANYPlayerStateStage* PS = ResolvePlayerState();
    if (!PS || !PS->CanControlPawn())
        return;

    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
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
    ANYPlayerStateStage* PS = ResolvePlayerState();
    if (!PS || !PS->CanControlPawn())
        return;

    if (HasAuthority())
    {
        PS->SetSprinting(true);
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
    const ANYPlayerStateStage* PS = ResolvePlayerState();
    if (!PS || !PS->CanControlPawn())
    {
        return;
    }

    if (!DefaultWeaponComp || !DefaultWeaponComp->CanSwapWeaponSlots())
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

void ANYCharacterPlayer::StartAttack()
{
    const ANYPlayerStateStage* PS = ResolvePlayerState();
    if (!PS || !PS->CanControlPawn() || !DefaultWeaponComp)
    {
        return;
    }

    ApplyAimDirection(GetLookAimDirection());

    if (HasAuthority())
    {
        DefaultWeaponComp->SetWantsToFire(true);
    }
    else
    {
        Server_SetWantsToFire(true, CachedAimDirection);
    }
}

void ANYCharacterPlayer::StopAttack()
{
    if (!DefaultWeaponComp)
    {
        return;
    }

    if (HasAuthority())
    {
        DefaultWeaponComp->SetWantsToFire(false);
    }
    else
    {
        Server_SetWantsToFire(false, CachedAimDirection);
    }
}

void ANYCharacterPlayer::Server_SetWantsToFire_Implementation(bool bWantsToFire, FVector_NetQuantizeNormal AimDir)
{
    if (ANYPlayerStateStage* PS = GetPlayerState<ANYPlayerStateStage>())
    {
        if (bWantsToFire && !PS->CanControlPawn())
        {
            return;
        }
    }

    ApplyAimDirection(AimDir);

    if (DefaultWeaponComp)
    {
        DefaultWeaponComp->SetWantsToFire(bWantsToFire);
    }
}

void ANYCharacterPlayer::Server_SetAimDirection_Implementation(FVector_NetQuantizeNormal AimDir)
{
    ApplyAimDirection(AimDir);
}

void ANYCharacterPlayer::UpdateLocalAim(float DeltaTime)
{
    if (!IsLocallyControlled())
    {
        return;
    }

    const ANYPlayerStateStage* PS = ResolvePlayerState();
    if (!PS || !PS->CanControlPawn())
    {
        return;
    }

    ApplyAimDirection(GetLookAimDirection());

    if (HasAuthority())
    {
        return;
    }

    AimRepTimer -= DeltaTime;
    if (AimRepTimer > 0.0f)
    {
        return;
    }

    AimRepTimer = AimRepInterval;
    Server_SetAimDirection(CachedAimDirection);
}

void ANYCharacterPlayer::ApplyAimDirection(const FVector& WorldDir)
{
    FVector FlatDir = WorldDir;
    FlatDir.Z = 0.0f;
    if (!FlatDir.Normalize())
    {
        return;
    }

    CachedAimDirection = FlatDir;

    if (HasAuthority() && DefaultWeaponComp)
    {
        DefaultWeaponComp->SetAimDirection(FlatDir);
    }
}

FVector ANYCharacterPlayer::GetLookAimDirection() const
{
    FVector Aim = FVector::ZeroVector;
    if (Controller)
    {
        Aim = Controller->GetControlRotation().Vector();
    }
    else
    {
        Aim = GetActorForwardVector();
    }

    Aim.Z = 0.0f;
    if (Aim.Normalize())
    {
        return Aim;
    }

    return GetActorForwardVector().GetSafeNormal2D();
}


// Stat
float ANYCharacterPlayer::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority() || !PS_ref)
        return 0.0f;

    PS_ref->ApplyDamage(DamageAmount);

    if (PS_ref->GetPlayerPhase() == ENYPlayerPhase::Alive)
    {
        const UWorld* World = GetWorld();
        const float Now = World ? World->GetTimeSeconds() : 0.0f;
        if (LastHitReactServerTime < 0.0f || Now - LastHitReactServerTime >= HitReactRetriggerDelay)
        {
            LastHitReactServerTime = Now;
            Multicast_OnHitFeedback(DamageAmount);
        }
    }

    return DamageAmount;
}

void ANYCharacterPlayer::Die()
{
    bIsDead = true;

    GetCharacterMovement()->DisableMovement();
    GetCharacterMovement()->StopMovementImmediately();

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    if (HasAuthority() && DefaultWeaponComp)
    {
        DefaultWeaponComp->SetWantsToFire(false);
    }

    // Attack montages occupy the slot until stopped; AnimGraph death waits otherwise.
    if (GetNetMode() != NM_DedicatedServer)
    {
        if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            AnimInstance->StopAllMontages(0.15f);
        }

        OnDeathFeedback();
    }
}

void ANYCharacterPlayer::Revive()
{
    bIsDead = false;

    GetCharacterMovement()->SetMovementMode(MOVE_Walking);

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    if (GetNetMode() != NM_DedicatedServer)
    {
        if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
        {
            AnimInstance->StopAllMontages(0.0f);
        }
    }
}

void ANYCharacterPlayer::ResolveMonsterSoftCollision()
{
    const ANYPlayerStateStage* PS = ResolvePlayerState();
    if (!PS || !PS->CanControlPawn())
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

void ANYCharacterPlayer::Multicast_OnHitFeedback_Implementation(float DamageTaken)
{
    if (GetNetMode() == NM_DedicatedServer || bIsDead)
    {
        return;
    }

    OnHitFeedback(DamageTaken);
}

void ANYCharacterPlayer::ResetWeaponForNewRun()
{
    if (DefaultWeaponComp)
    {
        DefaultWeaponComp->ResetWeaponLevel();
    }
}

void ANYCharacterPlayer::PlayAttackMontage(UAnimMontage* MontageToPlay)
{
    // Server
    if (!HasAuthority() || !MontageToPlay || bIsDead)
    {
        return;
    }

    Multicast_OnAttackStarted(MontageToPlay);
}

void ANYCharacterPlayer::UpdateWeaponVisual()
{
	if (!WeaponMeshComp || !DefaultWeaponComp)
	{
		return;
	}

	const UNYWeaponDefinition* Definition = DefaultWeaponComp->GetPrimarySlot().Definition;
	UStaticMesh* NewMesh = Definition ? Definition->WeaponMesh : nullptr;
	WeaponMeshComp->SetStaticMesh(NewMesh);
	RefreshHeldWeaponMeshVisibility();
}

void ANYCharacterPlayer::PushHeldWeaponMeshHidden()
{
	++HeldWeaponMeshHideCount;
	RefreshHeldWeaponMeshVisibility();
}

void ANYCharacterPlayer::PopHeldWeaponMeshHidden()
{
	HeldWeaponMeshHideCount = FMath::Max(0, HeldWeaponMeshHideCount - 1);
	RefreshHeldWeaponMeshVisibility();
}

void ANYCharacterPlayer::RefreshHeldWeaponMeshVisibility()
{
	if (WeaponMeshComp)
	{
		WeaponMeshComp->SetHiddenInGame(HeldWeaponMeshHideCount > 0);
	}
}

// NetMulticast
void ANYCharacterPlayer::Multicast_OnAttackStarted_Implementation(UAnimMontage* MontageToPlay)
{
    if (GetNetMode() == NM_DedicatedServer || !MontageToPlay || bIsDead)
    {
        return;
    }

    if (UAnimInstance* AnimInstance = GetMesh() ? GetMesh()->GetAnimInstance() : nullptr)
    {
        AnimInstance->Montage_Play(MontageToPlay);
    }
}
