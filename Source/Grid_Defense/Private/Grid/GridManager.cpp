#include "Grid/GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h" 
#include "Tower/TowerBase.h"
#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"
#include "GameMode/GridGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Nexus/Nexus.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;

	FloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISM"));
	SetRootComponent(FloorISM); 

	ObstacleISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ObstacleISM"));
	ObstacleISM->SetupAttachment(RootComponent);
	
	StartISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("StartISM"));
	StartISM->SetupAttachment(RootComponent);

	EndISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("EndISM"));
	EndISM->SetupAttachment(RootComponent);
}

FVector AGridManager::GetTileWorldPosition(int32 X, int32 Y) const
{
	int32 Index = GetIndex(X, Y); 
    
	// 안전 검사: 배열 범위 안에 있으면 월드 좌표 반환!
	if (GridArray.IsValidIndex(Index))
	{
		return GridArray[Index].WorldPosition;
	}
    
	return FVector::ZeroVector;
}

void AGridManager::UpdateFlowField()
{
	// 1. 초기화: 모든 타일의 비용을 최대로, 방향은 제로로 설정
	for (int i = 0; i < GridArray.Num(); ++i)
	{
		GridArray[i].FlowCost = 99999;
		GridArray[i].FlowDirection = FVector::ZeroVector;
	}

	TQueue<FIntPoint> Queue;
	// 목적지(넥서스) 설정 (보통 그리드의 우측 상단 끝)
	int32 NexusX = GridWidth - 1;
	int32 NexusY = GridHeight - 1;
	int32 NexusIndex = GetIndex(NexusX, NexusY);

	// 목적지의 비용은 0
	GridArray[NexusIndex].FlowCost = 0;
	Queue.Enqueue(FIntPoint(NexusX, NexusY));

	// 2. Integration Field: BFS를 이용해 모든 타일에서 목적지까지의 거리(Cost) 계산
	while (!Queue.IsEmpty())
	{
		FIntPoint CurrentNode;
		Queue.Dequeue(CurrentNode);

		int32 CurrentIndex = GetIndex(CurrentNode.X, CurrentNode.Y);
		int32 CurrentCost = GridArray[CurrentIndex].FlowCost;

		// 갈 수 있는 이웃 타일들 체크
		TArray<FIntPoint> Neighbors = GetWalkableNeighbors(CurrentNode.X, CurrentNode.Y);
		for (auto Neighbor : Neighbors)
		{
			int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
			
			// 아직 방문하지 않았거나, 더 짧은 경로를 발견한 경우
			if (GridArray[NeighborIndex].FlowCost > CurrentCost + 1)
			{
				GridArray[NeighborIndex].FlowCost = CurrentCost + 1;
				Queue.Enqueue(Neighbor);
			}
		}
	}

	// 3. Flow Field 생성: 각 타일에서 어느 방향으로 가야 최단거리인지 벡터 계산
	for (int i = 0; i < GridArray.Num(); ++i)
	{
		// 길이 없거나(99999) 이미 목적지(0)인 곳은 계산할 필요 없음
		if (GridArray[i].FlowCost >= 99999 || GridArray[i].FlowCost == 0)
		{
			continue;
		}

		TArray<FIntPoint> Neighbors = GetWalkableNeighbors(GridArray[i].X, GridArray[i].Y);

		int32 BestCost = GridArray[i].FlowCost;
		FIntPoint BestNeighborPos = FIntPoint(GridArray[i].X, GridArray[i].Y);
		bool bFoundPath = false;

		for (auto Neighbor : Neighbors)
		{
			int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
			int32 NeighborCost = GridArray[NeighborIndex].FlowCost;

			if (NeighborCost < BestCost)
			{
				BestCost = NeighborCost;
				BestNeighborPos = Neighbor; // 그 타일의 좌표를 저장!
				bFoundPath = true;
			}
		}

		// 4. 방향 벡터 저장 (나침반 굽기)
		if (bFoundPath)
		{
			FVector CurrentPos = GridArray[i].WorldPosition;
			FVector TargetPos = GridArray[GetIndex(BestNeighborPos.X, BestNeighborPos.Y)].WorldPosition;

			GridArray[i].FlowDirection = (TargetPos - CurrentPos).GetSafeNormal();
		}
	}

}

FVector AGridManager::GetFlowDirection(FVector WorldLocation) const
{
	FIntPoint GridPt = GetGridPointFromWorld(WorldLocation);
	int32 Index = GetIndex(GridPt.X, GridPt.Y);

	// 2. 안전 검사 후 해당 타일의 화살표 반환
	if (GridArray.IsValidIndex(Index))
	{
		return GridArray[Index].FlowDirection;
	}

	return FVector::ZeroVector;
}

FIntPoint AGridManager::GetGridPointFromWorld(FVector WorldLocation) const
{
	FVector LocalPosition = WorldLocation - GetActorLocation();

	int32 X = FMath::RoundToInt(LocalPosition.X / TileSize);
	int32 Y = FMath::RoundToInt(LocalPosition.Y / TileSize);

	X = FMath::Clamp(X, 0, GridWidth - 1);
	Y = FMath::Clamp(Y, 0, GridHeight - 1);
	return FIntPoint(X, Y);
	
}

int32 AGridManager::GetFlowCost(FVector WorldLocation) const
{
	FIntPoint GridPt = GetGridPointFromWorld(WorldLocation);
	int32 Index = GetIndex(GridPt.X, GridPt.Y);

	if (GridArray.IsValidIndex(Index))
	{
		return GridArray[Index].FlowCost;
	}

	return 999999;
}

void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateGrid();
}

void AGridManager::GenerateGrid()
{
    int32 TotalTiles = GridWidth * GridHeight;
    FVector ManagerLocation = GetActorLocation();

    bool bIsValidMap = false;
    int32 SafetyCounter = 0;

    // 💡 1. 길이 뚫린 맵이 나올 때까지 반복해서 맵 데이터를 생성합니다.
    while (!bIsValidMap && SafetyCounter < 100)
    {
        SafetyCounter++;

        // 배열 초기화
        GridArray.Empty();
        GridArray.SetNum(TotalTiles);

        // 메모리 상에서만 맵 데이터 생성 (화면엔 아직 안 그림)
        for (int32 Y = 0; Y < GridHeight; ++Y)
        {
            for (int32 X = 0; X < GridWidth; ++X)
            {
                int32 Index = GetIndex(X, Y);
                FGridInfo& Node = GridArray[Index];

                Node.X = X;
                Node.Y = Y;
                Node.WorldPosition = ManagerLocation + FVector(X * TileSize, Y * TileSize, 0.f);

                if (X == 0 && Y == 0) Node.TileType = ETileType::Start;
                else if (X == GridWidth - 1 && Y == GridHeight - 1) Node.TileType = ETileType::End;
                else if (FMath::FRandRange(0.f, 100.f) < 15.f) Node.TileType = ETileType::Rock;
                else Node.TileType = ETileType::Empty;

                Node.bIsWalkable = (Node.TileType != ETileType::Rock);
            }
        }

        // 플로우 필드를 돌려봅니다.
        UpdateFlowField();

        // 💡 2. 검증: 스포너(0,0) 위치에서 출구까지 갈 길이 있는가?
        if (GridArray[GetIndex(0, 0)].FlowCost < 99999)
        {
            bIsValidMap = true; // 합격! 루프를 탈출합니다.
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("길이 막힌 맵 생성됨! 재시도 중... (시도 횟수: %d)"), SafetyCounter);
        }
    }

    // 💡 3. 검증을 통과한 '완벽한 맵'을 드디어 화면에 그립니다.
    FloorISM->ClearInstances();
    ObstacleISM->ClearInstances();
    StartISM->ClearInstances();
    EndISM->ClearInstances();

    for (int32 Y = 0; Y < GridHeight; ++Y)
    {
        for (int32 X = 0; X < GridWidth; ++X)
        {
            int32 Index = GetIndex(X, Y);
            FGridInfo& Node = GridArray[Index];

            FVector RelativePos = FVector(X * TileSize, Y * TileSize, 0.f);
            FTransform TileTransform(RelativePos); 

            switch (Node.TileType)
            {
                case ETileType::Empty:    FloorISM->AddInstance(TileTransform); break;
                case ETileType::Rock:     ObstacleISM->AddInstance(TileTransform); break;
                case ETileType::Start:    StartISM->AddInstance(TileTransform); break;
                case ETileType::End:      EndISM->AddInstance(TileTransform); break;
            }
        }
    }

    // 4. 스포너와 넥서스 생성 (기존 코드 그대로)
    if (SpawnerClass)
    {
       FVector StartLoc = GridArray[GetIndex(0, 0)].WorldPosition;
       StartLoc.Z += 500.f; 
       FActorSpawnParameters SpawnParams;
       SpawnParams.Owner = this;

       ActiveSpawner = GetWorld()->SpawnActor<AEnemySpawner>(SpawnerClass, StartLoc, FRotator::ZeroRotator, SpawnParams);

       if (ActiveSpawner)
       {
          FVector EndLoc = GridArray[GetIndex(GridWidth - 1, GridHeight - 1)].WorldPosition;
          ActiveSpawner->SetTargetLocation(EndLoc);
       }
    }

    if (NexusClass)
    {
       FVector NexusLoc = GridArray[GetIndex(GridWidth - 1, GridHeight - 1)].WorldPosition;
       NexusLoc.Z += 500.f;

       ANexus* SpawnedNexus = GetWorld()->SpawnActor<ANexus>(NexusClass, NexusLoc, FRotator::ZeroRotator);
    }
    
    // 💡 주의: UpdateFlowField(); 는 이미 while문 안에서 마지막으로 합격했을 때 불렸으므로, 여기서 또 부를 필요가 없습니다!
}

void AGridManager::AddTower(int32 X, int32 Y, UTowerData* SelectedData)
{
	if (!SelectedData) return;
    
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return;

	if (!bIsTileBuildable(X, Y))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 이미 장애물이나 타워가 있습니다."));
		return;
	}
	
	GridArray[Index].bIsWalkable = false;

	UpdateFlowField();
	
	int32 SpawnerIndex = GetIndex(0, 0);
	bool bIsPathClear = (GridArray[SpawnerIndex].FlowCost < 99999);


	if (!bIsPathClear)
	{
		GridArray[Index].bIsWalkable = true; // 다시 길 열어주기
		UpdateFlowField(); // 필드 복구
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 길이 막힙니다!"));
		return;
	}
	
	TSubclassOf<AActor> ClassToSpawn = SelectedData->TowerActorClass;
	if (!ClassToSpawn) return;

	AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		int32 TowerCost = SelectedData->BuildCost;

		if (!GM->SpendGold(TowerCost))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("잔액 부족! 타워를 건설할 수 없습니다."));
			return;
		}
	}
	FVector SpawnLocation = GridArray[Index].WorldPosition + FVector(0.f, 0.f, 50.f);
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator);

	if (ATowerBase* Tower = Cast<ATowerBase>(SpawnedActor))
	{
		Tower->InitTower(SelectedData, false); // 실제 타워 모드
	}

	GridArray[Index].TileType = ETileType::Tower;
	GridArray[Index].bIsWalkable = false; 
	
	UpdateFlowField();
	
	UE_LOG(LogTemp, Warning, TEXT("타워 건설 완료: 모든 몬스터의 경로를 갱신합니다."));
}

bool AGridManager::bIsTileBuildable(int32 X, int32 Y) const
{
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return false;
	return GridArray[Index].TileType == ETileType::Empty;
}

TArray<FIntPoint> AGridManager::GetWalkableNeighbors(int32 X, int32 Y)
{
	TArray<FIntPoint> Neighbors;

	// 방향 설정
	FIntPoint Directions[4] = { FIntPoint(0, 1), FIntPoint(0 , -1), FIntPoint(1, 0), FIntPoint(-1, 0)};

	for (const auto Dir : Directions)
	{
		int32 CheckX = X + Dir.X;
		int32 CheckY = Y + Dir.Y;
		int32 Index = GetIndex(CheckX, CheckY);

		if (CheckX >= 0 && CheckX < GridWidth && CheckY >= 0 && CheckY < GridHeight)
		{
			if (GridArray.IsValidIndex(Index) && GridArray[Index].bIsWalkable)
			{
				Neighbors.Add(FIntPoint(CheckX, CheckY));
			}
		}
	}
	return Neighbors;
}

void AGridManager::DrawDebugFlowField()
{
	// 1. 모든 타일을 하나씩 검사합니다.
	for (const FGridInfo& Info : GridArray)
	{
		// 2. 방향 값이 있는 타일만 골라냅니다 (벽이나 목적지는 제외)
		if (!Info.FlowDirection.IsZero())
		{
			// 3. 언리얼 엔진의 '화살표 그리기' 기능을 실행합니다.
			DrawDebugDirectionalArrow(
				GetWorld(), 
				Info.WorldPosition, // 화살표 시작점 (타일 중심)
				Info.WorldPosition + (Info.FlowDirection * 50.f), // 화살표 끝점 (방향대로 50cm 뻗음)
				20.f,               // 화살촉 크기
				FColor::Yellow,     // 색깔 (노란색)
				false,              // 영구 지속 여부 (false면 금방 사라짐)
				10.f,               // 지속 시간 (-1은 보통 1프레임)
				0,                  // 우선순위
				2.f                 // 선 두께
			);
		}
	}
}