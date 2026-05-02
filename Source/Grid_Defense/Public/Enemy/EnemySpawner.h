#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveChanged, int32, CurrentWave);

USTRUCT(BlueprintType)
struct FEnemyGroupData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName EnemyRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SpawnCount = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnInterval = 1.0f;
};

USTRUCT(BlueprintType)
struct FWaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<FEnemyGroupData> EnemyGroups;
};

UCLASS()
class GRID_DEFENSE_API AEnemySpawner : public AActor
{
    GENERATED_BODY()
    
public: 
    AEnemySpawner();
    void SetTargetLocation(FVector InLoc) { TargetLocation = InLoc; }

    UFUNCTION()
    void OnEnemyDefeated();

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnWaveChanged OnWaveChanged;

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TObjectPtr<USkeletalMeshComponent> SpawnerMesh;
    
    UPROPERTY(EditAnywhere, Category = "Spawning")
    TArray<FWaveData> WaveList;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
    int32 CurrentSpawnLevel = 0;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    float TimeBetweenWaves = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    float FirstWaveDelay = 2.0f;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TObjectPtr<AActor> TargetDestination;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    TObjectPtr<UDataTable> EnemyDataTable;

    FTimerHandle SpawnTimerHandle;

    int32 CurrentGroupIndex = 0;
    int32 EnemySpawnInCurrentGroup = 0;
    int32 TotalEnemiesToSpawnInWave = 0;
    int32 TotalEnemiesSpawnedInWave = 0;

    void SpawnNextWave();
    void SpawnEnemy();

private:
    // [수정] private으로 이동 — 외부 직접 수정 방지
    int32 AliveEnemyCount = 0;

    FVector TargetLocation;
};