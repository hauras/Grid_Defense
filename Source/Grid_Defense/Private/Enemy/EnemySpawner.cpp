#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "State/GridGameState.h"

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpawnerMesh"));
    RootComponent = SpawnerMesh;

    CurrentSpawnLevel = 0;
    AliveEnemyCount = 0;
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, FirstWaveDelay, false);
}

void AEnemySpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
}

void AEnemySpawner::SpawnNextWave()
{
    if (!WaveList.IsValidIndex(CurrentSpawnLevel)) return;

    CurrentGroupIndex = 0;
    EnemySpawnInCurrentGroup = 0;
    TotalEnemiesSpawnedInWave = 0;
    AliveEnemyCount = 0;

    FWaveData& CurrentWave = WaveList[CurrentSpawnLevel];

    TotalEnemiesToSpawnInWave = 0;
    for (const FEnemyGroupData& Group : CurrentWave.EnemyGroups)
    {
        TotalEnemiesToSpawnInWave += Group.SpawnCount;
    }

    OnWaveChanged.Broadcast(CurrentSpawnLevel + 1);

    if (CurrentWave.EnemyGroups.Num() > 0)
    {
        float FirstInterval = CurrentWave.EnemyGroups[0].SpawnInterval;
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, FirstInterval, true);
    }
}

void AEnemySpawner::SpawnEnemy()
{
    if (!WaveList.IsValidIndex(CurrentSpawnLevel)) return;
    FWaveData& CurrentWave = WaveList[CurrentSpawnLevel];

    if (!CurrentWave.EnemyGroups.IsValidIndex(CurrentGroupIndex)) return;
    FEnemyGroupData& CurrentGroup = CurrentWave.EnemyGroups[CurrentGroupIndex];

    if (!EnemyDataTable) return;

    static const FString ContextString(TEXT("Spawn Enemy Context"));
    FEnemyData* EnemyData = EnemyDataTable->FindRow<FEnemyData>(CurrentGroup.EnemyRowName, ContextString);

    if (EnemyData && EnemyData->EnemyClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(
            EnemyData->EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);

        if (SpawnedEnemy)
        {
            SpawnedEnemy->InitializeEnemy(CurrentGroup.EnemyRowName);

            EnemySpawnInCurrentGroup++;
            TotalEnemiesSpawnedInWave++;
            AliveEnemyCount++;

            if (EnemySpawnInCurrentGroup >= CurrentGroup.SpawnCount)
            {
                GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

                CurrentGroupIndex++;
                EnemySpawnInCurrentGroup = 0;

                if (CurrentWave.EnemyGroups.IsValidIndex(CurrentGroupIndex))
                {
                    float NextInterval = CurrentWave.EnemyGroups[CurrentGroupIndex].SpawnInterval;
                    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, NextInterval, true);
                }
            }
        }
    }
}

void AEnemySpawner::OnEnemyDefeated()
{
    AliveEnemyCount = FMath::Max(0, AliveEnemyCount - 1);

    if (!WaveList.IsValidIndex(CurrentSpawnLevel)) return;

    if (TotalEnemiesSpawnedInWave >= TotalEnemiesToSpawnInWave && AliveEnemyCount <= 0)
    {
        CurrentSpawnLevel++;

        if (AGridGameState* GS = Cast<AGridGameState>(GetWorld()->GetGameState()))
        {
            GS->ShowCardSelectUI();
        }

        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, TimeBetweenWaves, false);
    }
}