
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EnemySpawner.generated.h"

class AEnemyBase;
USTRUCT(BlueprintType)
struct FSpawnWaveData
{
	GENERATED_BODY()

public:
	// 어떤 블루프린트를 소환할 것인가? (BP_Enemy)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<AEnemyBase> EnemyClass;

	// 데이터 테이블에서 어떤 Row를 적용할 것인가? (Dragon_Fire, Dragon_Ice 등)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EnemyRowName;

	// (선택 사항) 이 몬스터가 나올 확률이나 무게감 등등 추가 가능
};

UCLASS()
class GRID_DEFENSE_API AEnemySpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	AEnemySpawner();
	void SetTargetLocation(FVector InLoc) { TargetLocation = InLoc; }
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	TObjectPtr<USkeletalMeshComponent> SpawnerMesh;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	TArray<FSpawnWaveData> EnemyList;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
	int32 CurrentSpawnLevel = 0;
	
	UPROPERTY(EditAnywhere, Category = "Spawning")
	float SpawnInterval = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Spawning")
	AActor* TargetDestination;

	FTimerHandle SpawnTimerHandle;

	void SpawnEnemy();
private:

	FVector TargetLocation;
};
