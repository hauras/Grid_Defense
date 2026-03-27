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

		// 💡 정빈님이 작성하려던 Mincost 로직의 완성형입니다.
		for (auto Neighbor : Neighbors)
		{
			int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
			int32 NeighborCost = GridArray[NeighborIndex].FlowCost;

			// 내 주변 타일 중 나보다 숫자가 낮은(목적지에 더 가까운) 타일을 찾습니다.
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

			// 💡 방향 = (가야 할 타일 위치 - 현재 타일 위치)
			// 이 벡터를 통해 몬스터가 어느 방향으로 힘을 받아야 하는지 결정됩니다.
			GridArray[i].FlowDirection = (TargetPos - CurrentPos).GetSafeNormal();
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Flow Field 생성 완료! 모든 드래곤이 지도를 갱신했습니다."));
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

void AGridManager::BeginPlay()
{
	Super::BeginPlay();
	GenerateGrid();
}

void AGridManager::GenerateGrid()
{
	int32 TotalTiles = GridWidth * GridHeight;
	GridArray.Empty();
	GridArray.SetNum(TotalTiles);

	FloorISM->ClearInstances();
	ObstacleISM->ClearInstances();
	StartISM->ClearInstances();
	EndISM->ClearInstances();

	FVector ManagerLocation = GetActorLocation();

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			int32 Index = GetIndex(X, Y);
			FGridInfo& Node = GridArray[Index];

			Node.X = X;
			Node.Y = Y;
          
			// 메시 피벗이 중앙일 때: (X * TileSize)가 타일의 중심
			Node.WorldPosition = ManagerLocation + FVector(X * TileSize, Y * TileSize, 0.f);

			if (X == 0 && Y == 0) Node.TileType = ETileType::Start;
			else if (X == GridWidth - 1 && Y == GridHeight - 1) Node.TileType = ETileType::End;
			else if (FMath::FRandRange(0.f, 100.f) < 15.f) Node.TileType = ETileType::Rock;
			else Node.TileType = ETileType::Empty;

			Node.bIsWalkable = (Node.TileType != ETileType::Rock);

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

	UpdateFlowField();
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

	// A* 알고리즘 용 변수
	/*int32 SpawnerX = 0;
	int32 SpawnerY = 0;
	int32 NexusX = GridWidth - 1; 
	int32 NexusY = GridHeight - 1; 

	TArray<FIntPoint> DummyPath; 
    
	bool bIsPathClear = FindPath(SpawnerX, SpawnerY, NexusX, NexusY, DummyPath);*/
	int32 SpawnerIndex = GetIndex(0, 0);
	bool bIsPathClear = (GridArray[SpawnerIndex].FlowCost < 99999);

	//GridArray[Index].bIsWalkable = true;

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

	/*TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);

	for (AActor* Actor : FoundEnemies)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
        
		if (Enemy && !Enemy->IsDead())
		{
			Enemy->RecalculatePath(); 
		}
	}*/
	UpdateFlowField();
	
	UE_LOG(LogTemp, Warning, TEXT("타워 건설 완료: 모든 몬스터의 경로를 갱신합니다."));
}

bool AGridManager::bIsTileBuildable(int32 X, int32 Y) const
{
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return false;
	return GridArray[Index].TileType == ETileType::Empty;
}

/*int32 AGridManager::GetDistance(int32 NodeAX, int32 NodeAY, int32 NodeBX, int32 NodeBY)
{
	int32 X = FMath::Abs(NodeAX - NodeBX);
	int32 Y = FMath::Abs(NodeAY - NodeBY);

	return (X + Y) * 10;
}*/

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

/*
bool AGridManager::FindPath(int32 StartX, int32 StartY, int32 EndX, int32 EndY, TArray<FIntPoint>& OutPath)
{
    if (!GridArray.IsValidIndex(GetIndex(StartX, StartY)) || !GridArray.IsValidIndex(GetIndex(EndX, EndY))) return false;

// 💡 여기서부터 A* 알고리즘 로직 비활성화 (나중에 비교를 위해 보존)
#if 0 
    TArray<FAStarNode*> OpenList; // 앞으로 볼 노드
    TArray<FIntPoint> ClosedList; // 방문한 노드
    TArray<FAStarNode*> AllNodes;

    FAStarNode* StartNode = new FAStarNode(StartX, StartY);
    AllNodes.Add(StartNode);
    OpenList.Add(StartNode);

    while (OpenList.Num() > 0)
    {
       FAStarNode* CurrentNode = OpenList[0];
       int32 CurrentIndex = 0;

       for (int32 i = 1; i < OpenList.Num(); ++i)
       {
          if (OpenList[i]->FCost < CurrentNode->FCost || (OpenList[i]->FCost == CurrentNode->FCost && OpenList[i]->HCost < CurrentNode->HCost))
          {
             CurrentNode = OpenList[i];
             CurrentIndex = i;
          }
       }

       OpenList.RemoveAt(CurrentIndex);
    
       ClosedList.Add(FIntPoint(CurrentNode->X, CurrentNode->Y));

       // 도착지 확인
       if (CurrentNode->X == EndX && CurrentNode->Y == EndY)
       {
          OutPath.Empty();
          FAStarNode* TraceNode = CurrentNode;
          while (TraceNode != nullptr)
          {
             OutPath.Add(FIntPoint(TraceNode->X, TraceNode->Y));
             TraceNode = TraceNode->ParentNode;
          }

          Algo::Reverse(OutPath);
          
          for (auto Node : AllNodes)
          {
             delete Node;
          }
          return true;
       }

       TArray<FIntPoint> Neighbors = GetWalkableNeighbors(CurrentNode->X, CurrentNode->Y);

       for (auto Neighbor : Neighbors)
       {
          if (ClosedList.Contains(Neighbor))
          {
             continue;
          }

          int32 NewGCost = CurrentNode->GCost + GetDistance(CurrentNode->X, CurrentNode->Y, Neighbor.X, Neighbor.Y);

          FAStarNode* NeighborNode = nullptr;

          for (FAStarNode* Node : OpenList)
          {
             if (Node->X == Neighbor.X && Node->Y == Neighbor.Y)
             {
                NeighborNode = Node;
                break;
             }
          }

          if (NeighborNode == nullptr || NewGCost < NeighborNode->GCost)
          {
             if (NeighborNode == nullptr)
             {
                NeighborNode = new FAStarNode(Neighbor.X, Neighbor.Y);
                AllNodes.Add(NeighborNode);
                OpenList.Add(NeighborNode);
             }

             // G 최신화
             NeighborNode->GCost = NewGCost;
             // H 계산
             NeighborNode->HCost = GetDistance(NeighborNode->X, NeighborNode->Y, EndX, EndY);
             // F 계산 (G + H)
             NeighborNode->FCost = NeighborNode->GCost + NeighborNode->HCost;
             
             NeighborNode->ParentNode = CurrentNode;
          }
       }
    }

    for (auto Node : AllNodes)
    {
       delete Node;
    }
#endif

    // 플로우 필드를 완성하기 전까지 임시로 true를 반환하여 에러 방지
    return true; 
}
*/
