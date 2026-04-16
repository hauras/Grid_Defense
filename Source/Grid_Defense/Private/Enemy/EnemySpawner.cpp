
#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"

AEnemySpawner::AEnemySpawner()
{
	PrimaryActorTick.bCanEverTick = false;

	SpawnerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpawnerMesh"));
	RootComponent = SpawnerMesh;

	CurrentSpawnLevel = 0;
	EnemySpawnInCurrentWave = 0;
}

void AEnemySpawner::BeginPlay()
{
	Super::BeginPlay();

	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, 2.0f, true);
	
}

void AEnemySpawner::SpawnNextWave()
{
	if (!EnemyList.IsValidIndex(CurrentSpawnLevel))
	{
		UE_LOG(LogTemp, Warning, TEXT("모든 웨이브 스폰이 완료되었습니다!"));
		return;
	}

	EnemySpawnInCurrentWave = 0;
	FSpawnWaveData CurrentWave = EnemyList[CurrentSpawnLevel];

	UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 시작! 총 %d 마리 소환 예정"), CurrentSpawnLevel + 1, CurrentWave.SpawnCount);

	// 해당 웨이브의 SpawnInterval 간격으로 SpawnEnemy 함수 반복 실행
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, CurrentWave.SpawnInterval, true);
}

void AEnemySpawner::SpawnEnemy()
{
	if (!EnemyList.IsValidIndex(CurrentSpawnLevel)) return;

	FSpawnWaveData Selected = EnemyList[CurrentSpawnLevel];

	// 🚨 1. 에디터에서 데이터 테이블이 잘 들어있는지 확인!
	if (!EnemyDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("스포너에 EnemyDataTable이 연결되지 않았습니다!"));
		return;
	}

	// 2. 웨이브에 적힌 행 이름(EnemyRowName)으로 적 데이터 테이블(FEnemyData)을 검색합니다.
	static const FString ContextString(TEXT("Spawn Enemy Context"));
	FEnemyData* EnemyData = EnemyDataTable->FindRow<FEnemyData>(Selected.EnemyRowName, ContextString);

	// 3. 데이터를 성공적으로 찾았고, 그 데이터 안에 EnemyClass(보스인지 일반몹인지)가 있다면?
	if (EnemyData && EnemyData->EnemyClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		// 💡 4. 무조건 스폰하는 게 아니라, 데이터에 적힌 클래스(EnemyClass)로 소환합니다!
		AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyData->EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);

		if (SpawnedEnemy)
		{
			// 스폰된 녀석에게 "너는 Boss_01번 데이터니까 체력 5000으로 셋팅해!" 라고 지시
			SpawnedEnemy->InitializeEnemy(Selected.EnemyRowName); 
            
			// 소환 카운트 증가 로직
			EnemySpawnInCurrentWave++;

			if (EnemySpawnInCurrentWave >= Selected.SpawnCount)
			{
				GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
				CurrentSpawnLevel++;
				UE_LOG(LogTemp, Warning, TEXT("웨이브 종료! 다음 웨이브 준비..."));
				GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, TimeBetweenWaves, false);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("데이터 테이블에서 %s 를 찾을 수 없거나 EnemyClass가 비어있습니다!"), *Selected.EnemyRowName.ToString());
	}
}

