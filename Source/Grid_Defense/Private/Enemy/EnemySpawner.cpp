
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

    if (Selected.EnemyClass)
    {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

        AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(Selected.EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);

        if (SpawnedEnemy)
        {
            SpawnedEnemy->InitializeEnemy(Selected.EnemyRowName); 

            // ==========================================
            // [A* 내비게이션 경로 주입 로직 - 기존 유지]
            // ==========================================
            AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
            if (GridManager)
            {
                TArray<FIntPoint> GridPath;
                int32 StartX = 0, StartY = 0;
                int32 EndX = GridManager->GetGridWidth() - 1; 
                int32 EndY = GridManager->GetGridHeight() - 1;

                if (GridManager->FindPath(StartX, StartY, EndX, EndY, GridPath))
                {
                    TArray<FVector> WorldPath;
                    for (FIntPoint Node : GridPath)
                    {
                        FVector NodeWorldPos = GridManager->GetTileWorldPosition(Node.X, Node.Y);
                        NodeWorldPos.Z += 100.f; 
                        WorldPath.Add(NodeWorldPos);
                    }
                    SpawnedEnemy->SetPath(WorldPath);
                }
            }

            // 💡 [핵심 추가] 소환 카운트 1 증가
            EnemySpawnInCurrentWave++;

            // 목표 마리 수만큼 다 소환했는지 체크
            if (EnemySpawnInCurrentWave >= Selected.SpawnCount)
            {
                // 1. 반복 소환 타이머 정지
                GetWorldTimerManager().ClearTimer(SpawnTimerHandle);

                // 2. 다음 웨이브로 레벨 업
                CurrentSpawnLevel++;

                UE_LOG(LogTemp, Warning, TEXT("웨이브 종료! %f초 대기 후 다음 웨이브 시작..."), TimeBetweenWaves);

                // 3. 쉬는 시간(TimeBetweenWaves) 이후에 다음 웨이브 시작 함수 호출
                GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, TimeBetweenWaves, false);
            }
        }
    }
}

