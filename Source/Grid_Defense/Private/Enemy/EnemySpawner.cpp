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
    EnemySpawnInCurrentWave = 0;
    AliveEnemyCount = 0; // 💡 살아있는 몬스터 수 초기화
}

void AEnemySpawner::BeginPlay()
{
    Super::BeginPlay();

    // 게임 시작 후 2초 뒤에 첫 웨이브 시작
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
    AliveEnemyCount = 0; // 안전을 위해 다음 웨이브 시작 시 0으로 초기화
    
    FSpawnWaveData CurrentWave = EnemyList[CurrentSpawnLevel];

    UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 시작! 총 %d 마리 소환 예정"), CurrentSpawnLevel + 1, CurrentWave.SpawnCount);

    // 해당 웨이브의 SpawnInterval 간격으로 SpawnEnemy 함수 반복 실행
    GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnEnemy, CurrentWave.SpawnInterval, true);
}

void AEnemySpawner::SpawnEnemy()
{
    if (!EnemyList.IsValidIndex(CurrentSpawnLevel)) return;

    FSpawnWaveData Selected = EnemyList[CurrentSpawnLevel];

    if (!EnemyDataTable)
    {
       UE_LOG(LogTemp, Error, TEXT("스포너에 EnemyDataTable이 연결되지 않았습니다!"));
       return;
    }

    static const FString ContextString(TEXT("Spawn Enemy Context"));
    FEnemyData* EnemyData = EnemyDataTable->FindRow<FEnemyData>(Selected.EnemyRowName, ContextString);

    if (EnemyData && EnemyData->EnemyClass)
    {
       FActorSpawnParameters SpawnParams;
       SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

       AEnemyBase* SpawnedEnemy = GetWorld()->SpawnActor<AEnemyBase>(EnemyData->EnemyClass, GetActorLocation(), GetActorRotation(), SpawnParams);

       if (SpawnedEnemy)
       {
          SpawnedEnemy->InitializeEnemy(Selected.EnemyRowName); 
            
          // 💡 소환 카운트 증가 및 살아있는 몬스터 수 증가
          EnemySpawnInCurrentWave++;
          AliveEnemyCount++; 

          // 이번 웨이브의 목표 소환량을 다 채웠다면?
          if (EnemySpawnInCurrentWave >= Selected.SpawnCount)
          {
             // 💡 타이머만 정지하고 "다음 웨이브 준비"는 여기서 하지 않습니다! (적들이 아직 살아있으므로)
             GetWorldTimerManager().ClearTimer(SpawnTimerHandle);
             UE_LOG(LogTemp, Warning, TEXT("이번 웨이브 소환 완료! 적 전멸 대기 중..."));
          }
       }
    }
    else
    {
       UE_LOG(LogTemp, Error, TEXT("데이터 테이블에서 %s 를 찾을 수 없거나 EnemyClass가 비어있습니다!"), *Selected.EnemyRowName.ToString());
    }
}

// 💡 적이 죽을 때마다 호출될 함수
void AEnemySpawner::OnEnemyDefeated()
{
    AliveEnemyCount--; // 적이 한 마리 죽었으므로 카운트 감소

    if (!EnemyList.IsValidIndex(CurrentSpawnLevel)) return;

    // 목표 소환량을 다 소환했고 && 살아있는 적이 0마리라면 -> 진짜 웨이브 클리어!
    if (EnemySpawnInCurrentWave >= EnemyList[CurrentSpawnLevel].SpawnCount && AliveEnemyCount <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("웨이브 %d 클리어! 카드 보상 등장!"), CurrentSpawnLevel + 1);

        CurrentSpawnLevel++; // 다음 웨이브 레벨로 증가

        // 🌟 1. 카드 UI 띄우기 및 시간 정지
        if (AGridGameState* GS = Cast<AGridGameState>(GetWorld()->GetGameState()))
        {
            GS->ShowCardSelectUI(); 
        }

        // 🌟 2. 다음 웨이브 시작 타이머 세팅
        // (현재 UI가 뜨면서 TimeDilation이 0.0이 되므로 타이머가 흐르지 않습니다.
        // 플레이어가 카드를 선택해서 TimeDilation이 1.0이 되는 순간부터 타이머가 계산되어 다음 웨이브가 시작됩니다!)
        GetWorldTimerManager().SetTimer(SpawnTimerHandle, this, &AEnemySpawner::SpawnNextWave, TimeBetweenWaves, false);
    }
}