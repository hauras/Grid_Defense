
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

	// 💡 하나의 웨이브 안에 여러 종류의 몬스터를 넣을 수 있는 '배열'
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

	int32 AliveEnemyCount = 0;

	// 💡 몬스터가 죽을 때마다 호출해 줄 함수
	UFUNCTION()
	void OnEnemyDefeated();

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnWaveChanged OnWaveChanged;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TObjectPtr<USkeletalMeshComponent> SpawnerMesh;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<FWaveData> WaveList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 CurrentSpawnLevel = 0;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	float TimeBetweenWaves = 5.0f;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	AActor* TargetDestination;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	TObjectPtr<UDataTable> EnemyDataTable;
	
	
	FTimerHandle SpawnTimerHandle;

	int32 CurrentGroupIndex = 0;           // 현재 웨이브 안에서 몇 번째 그룹(소대)을 소환 중인가?
	int32 EnemySpawnInCurrentGroup = 0;    // 현재 그룹에서 몇 마리나 소환했는가?
    
	int32 TotalEnemiesToSpawnInWave = 0;   // 이번 웨이브 전체 목표 마릿수 (클리어 체크용)
	int32 TotalEnemiesSpawnedInWave = 0;

	void SpawnNextWave();
	void SpawnEnemy();
private:

	FVector TargetLocation;
};
