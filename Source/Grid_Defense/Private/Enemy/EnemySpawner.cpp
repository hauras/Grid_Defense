
#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"

// Sets default values
AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpawnerMesh"));
	RootComponent = SpawnerMesh;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, SpawnInterval, true);
	
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyList.IsValidIndex(CurrentSpawnLevel)) return;

	FSpawnWaveData Selected = EnemyList[CurrentSpawnLevel];

	if (!Selected.EnemyClass) return;

	// 스폰 실행
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(
	   Selected.EnemyClass, 
	   GetActorLocation(), 
	   GetActorRotation(),
	   SpawnParams // 👈 파라미터 꼭 전달!
	);

	if (SpawnedEnemy)
	{
		// 1. 데이터 주입 및 초기화
		SpawnedEnemy->EnemyDataRowName = Selected.EnemyRowName;
		SpawnedEnemy->InitializeStats(); 

		if (!TargetLocation.IsNearlyZero()) 
		{
			SpawnedEnemy->MoveToTarget(TargetLocation);
			UE_LOG(LogTemp, Warning, TEXT("스포너가 목적지(%s)로 이동 명령을 보냈음!"), *TargetLocation.ToString());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("목적지(TargetLocation)가 설정되지 않았습니다!"));
		}
	}
}

