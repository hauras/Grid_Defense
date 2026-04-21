#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "State/GridGameState.h" // 💡 카드 UI를 띄우기 위해 GameState 포함

AEnemySpawner::AEnemySpawner()
{
    PrimaryActorTick.bCanEverTick = false;

    SpawnerMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SpawnerMesh"));
    RootComponent = SpawnerMesh;

    CurrentSpawnLevel = 0;
    AliveEnemyCount = 0; // 💡 살아있는 몬스터 수 초기화
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 게임 시작 후 2초 뒤에 첫 웨이브 시작
	GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, 2.0f, false);
}

void AEnemySpawner::SpawnNextWave()
{
	if (!WaveList.IsValidIndex(CurrentSpawnLevel))
	{
		return;
	}

	
	// 💡 웨이브 시작 시 모든 추적 변수 초기화
	CurrentGroupIndex = 0;
	EnemySpawnInCurrentGroup = 0;
	TotalEnemiesSpawnedInWave = 0;
	AliveEnemyCount = 0; 
    
	FWaveData& CurrentWave = WaveList[CurrentSpawnLevel];

	// 💡 이번 웨이브에서 총 몇 마리를 잡아야 하는지 (모든 그룹의 소환량 합산) 계산
	TotalEnemiesToSpawnInWave = 0;
	for (const FEnemyGroupData& Group : CurrentWave.EnemyGroups)
	{
		TotalEnemiesToSpawnInWave += Group.SpawnCount;
	}
	OnWaveChanged.Broadcast(CurrentSpawnLevel + 1);
	UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 시작! 총 %d 마리 소환 예정"), CurrentSpawnLevel + 1, TotalEnemiesToSpawnInWave);

	// 첫 번째 그룹이 있다면 소환 시작!
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
    FEnemyGroupData& CurrentGroup = CurrentWave.EnemyGroups[CurrentGroupIndex]; // 현재 소환할 그룹

    if (!EnemyDataTable) return;

    static const FString ContextString(TEXT("Spawn Enemy Context"));
    FEnemyData* EnemyData = EnemyDataTable->FindRow<FEnemyData>(CurrentGroup.EnemyRowName, ContextString);

    if (EnemyData && EnemyData->EnemyClass)
    {
       FActorSpawnParameters SpawnParams;
       SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

       AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyData->EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);

       if (SpawnedEnemy)
       {
          SpawnedEnemy->InitializeEnemy(CurrentGroup.EnemyRowName); 
            
          // 카운트 증가
          EnemySpawnInCurrentGroup++;
          TotalEnemiesSpawnedInWave++;
          AliveEnemyCount++; 

          // 🌟 [핵심] 현재 '그룹'의 목표량을 다 채웠다면? -> 다음 그룹으로 넘어갑니다!
          if (EnemySpawnInCurrentGroup >= CurrentGroup.SpawnCount)
          {
             GetWorldTimerManager().ClearTimer(SpawnTimerHandle); // 현재 타이머 정지
             
             CurrentGroupIndex++;         // 다음 그룹으로 인덱스 이동
             EnemySpawnInCurrentGroup = 0; // 그룹 내 소환 카운트 초기화

             // 다음 그룹이 남아있다면, 그 그룹의 간격(Interval)으로 타이머 재시작!
             if (CurrentWave.EnemyGroups.IsValidIndex(CurrentGroupIndex))
             {
                 float NextInterval = CurrentWave.EnemyGroups[CurrentGroupIndex].SpawnInterval;
                 GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, NextInterval, true);
             }
             else
             {
                 UE_LOG(LogTemp, Warning, TEXT("이번 웨이브의 모든 몬스터 소환 완료! 적 전멸 대기 중..."));
             }
          }
       }
    }
}

void AEnemySpawner::OnEnemyDefeated()
{
	AliveEnemyCount--; 

	if (!WaveList.IsValidIndex(CurrentSpawnLevel)) return;

	// 🌟 [핵심] "웨이브 전체 소환량을 다 채웠고" && "살아있는 적이 0마리"라면 -> 웨이브 클리어!
	if (TotalEnemiesSpawnedInWave >= TotalEnemiesToSpawnInWave && AliveEnemyCount <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 클리어! 카드 보상 등장!"), CurrentSpawnLevel + 1);

		CurrentSpawnLevel++; 

		if (AGridGameState* GS = Cast<AGridGameState>(GetWorld()->GetGameState()))
		{
			GS->ShowCardSelectUI(); 
		}

		GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, TimeBetweenWaves, false);
	}
}