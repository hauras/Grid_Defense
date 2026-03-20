
#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"

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
		// 💡 수정 1: 직접 변수를 건드리는 대신, 아까 만든 Public 함수를 호출합니다.
		// 몬스터 스스로가 이름표를 달고 스탯을 초기화하게 합니다.
		SpawnedEnemy->InitializeEnemy(Selected.EnemyRowName); 

		// ==========================================
		// 💡 [A* 내비게이션 경로 넘겨주기]
		// ==========================================
      
		AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
      
		if (GridManager)
		{
			TArray<FIntPoint> GridPath;
        
			// 💡 수정 2: GridWidth, GridHeight 변수 대신 Getter 함수를 사용합니다.
			int32 StartX = 0; 
			int32 StartY = 0;
			int32 EndX = GridManager->GetGridWidth() - 1; 
			int32 EndY = GridManager->GetGridHeight() - 1;

			if (GridManager->FindPath(StartX, StartY, EndX, EndY, GridPath))
			{
				TArray<FVector> WorldPath;

				for (FIntPoint Node : GridPath)
				{
					// 💡 수정 3: GridArray에 직접 접근하는 대신 월드 좌표를 반환하는 함수를 사용합니다.
					FVector NodeWorldPos = GridManager->GetTileWorldPosition(Node.X, Node.Y);
         
					NodeWorldPos.Z += 50.f; // 몬스터가 땅에 안 파묻히게 살짝 띄워주기
					WorldPath.Add(NodeWorldPos);
				}

				SpawnedEnemy->SetPath(WorldPath);
				UE_LOG(LogTemp, Warning, TEXT("A* 내비게이션 경로 주입 완료!"));
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("길이 막혀서 경로를 주입하지 못했습니다!"));
			}
		}
	}
}

