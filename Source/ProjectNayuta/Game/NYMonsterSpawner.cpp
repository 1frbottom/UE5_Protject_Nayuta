// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYMonsterSpawner.h"

#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

#include "Game/NYGameMode.h"
#include "Game/NYGameStateBase.h"
#include "Player/NYPlayerStateBase.h"
#include "Characters/CharacterPlayers/NYCharacterPlayer.h"
#include "Characters/CharacterMonsters/NYMonsterBase.h"



ANYMonsterSpawner::ANYMonsterSpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnInterval = 5.0f;
    SpawnRadius = 500.0f;
}

void ANYMonsterSpawner::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority() && MonsterClass != nullptr)
    {
        if (ANYGameMode* GM = Cast<ANYGameMode>(GetWorld()->GetAuthGameMode()))
        {
            GM->RegisterSpawner(this);
        }

        //GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &ANYMonsterSpawner::SpawnMonsterRoutine, SpawnInterval, true);
    }
}

void ANYMonsterSpawner::StartSpawning()
{
    if (!HasAuthority() || !MonsterClass)
        return;

    GetWorld()->GetTimerManager().SetTimer(SpawnTimerHandle, this, &ANYMonsterSpawner::SpawnMonsterRoutine, SpawnInterval, true);
}

void ANYMonsterSpawner::StopSpawning()
{
    GetWorld()->GetTimerManager().ClearTimer(SpawnTimerHandle);
}

void ANYMonsterSpawner::UpdateSpawnerData(TSubclassOf<class ANYMonsterBase> NewMonsterClass, float NewInterval)
{
    if (NewMonsterClass)
        MonsterClass = NewMonsterClass;

    SpawnInterval = NewInterval;
}

void ANYMonsterSpawner::SpawnMonsterRoutine()
{
    // 1. 살아있는 플레이어 리스팅
    TArray<ANYCharacterPlayer*> AlivePlayers;

    if (ANYGameStateBase* GS = GetWorld()->GetGameState<ANYGameStateBase>())
        for (APlayerState* PS : GS->PlayerArray)
        {
            ANYPlayerStateBase* MyPS = Cast<ANYPlayerStateBase>(PS);

            if (MyPS && MyPS->GetPlayerPhase() == ENYPlayerPhase::Alive)
                if (ANYCharacterPlayer* Character = Cast<ANYCharacterPlayer>(MyPS->GetPawn()))
                {
                    AlivePlayers.Add(Character);
                }
        }

    if (AlivePlayers.IsEmpty())
        return;

    // 2. 한명 선택
    ANYCharacterPlayer* TargetCharacter = AlivePlayers[FMath::RandRange(0, AlivePlayers.Num() - 1)];

    // 3. 타겟 캐릭터 주변 랜덤 2D 원형 좌표 계산
    FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
    FVector SpawnLocation = TargetCharacter->GetActorLocation() + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);
    FRotator SpawnRotation = FRotator::ZeroRotator;

    // 4. 몬스터 스폰 
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

    // TODO : 나중에 Object Pool식으로 바꾸기
    ANYMonsterBase* SpawnedMonster = GetWorld()->SpawnActor<ANYMonsterBase>(MonsterClass, SpawnLocation, SpawnRotation, SpawnParams);

    // 5. 스폰된 몬스터에게 타겟 플레이어 지정 (클라이언트들에게 동기화)
    if (SpawnedMonster)
    {
        SpawnedMonster->SetTarget(TargetCharacter);
    }

}