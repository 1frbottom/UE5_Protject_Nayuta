// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/NYMonsterSpawner.h"

#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

#include "Game/NYGameModeStage.h"
#include "Game/NYGameStateStage.h"
#include "Game/NYMonsterPoolManager.h"

#include "Player/NYPlayerStateStage.h"

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
        if (ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode()))
        {
            GM->RegisterSpawner(this);
        }
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
            ANYPlayerStateStage* MyPS = Cast<ANYPlayerStateStage>(PS);

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

    // GM, 풀매니저
    ANYGameModeStage* GM = Cast<ANYGameModeStage>(GetWorld()->GetAuthGameMode());
    if (!GM || !GM->GetMonsterPoolManager())
        return;

    ANYMonsterPoolManager* PoolManager = GM->GetMonsterPoolManager();

    // 3. SpawnCountPerTick 만큼 반복해서 풀에서 꺼내기 (Batch Spawning)
    for (int32 i = 0; i < SpawnCountPerTick; i++)
    {
        FVector2D RandomCircle = FMath::RandPointInCircle(SpawnRadius);
        FVector SpawnLocation = TargetCharacter->GetActorLocation() + FVector(RandomCircle.X, RandomCircle.Y, 0.0f);

        // 캡슐 콜리전 겹침 방지를 위한 임시 오프셋 (인덱스 기반으로 흩뿌리기)
        SpawnLocation.X += FMath::RandRange(-50.0f, 50.0f) * i;
        SpawnLocation.Y += FMath::RandRange(-50.0f, 50.0f) * i;
        SpawnLocation.Z += 50.0f; // 바닥 관통 방지

        FRotator SpawnRotation = FRotator::ZeroRotator;

        // 풀에서 몬스터 가져오기 (SpawnActor 대체)
        ANYMonsterBase* SpawnedMonster = PoolManager->GetMonster(SpawnLocation, SpawnRotation);

        // 타겟 지정
        if (SpawnedMonster)
        {
            SpawnedMonster->SetTarget(TargetCharacter);
        }
    }


}