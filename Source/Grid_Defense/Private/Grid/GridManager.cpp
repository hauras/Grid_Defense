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
		StartLoc.Z += 55.f;

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
		NexusLoc.Z += 55.f;

		ANexus* SpawnedNexus = GetWorld()->SpawnActor<ANexus>(NexusClass, NexusLoc, FRotator::ZeroRotator);
	}
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
	
	// 테스트용
	GridArray[Index].bIsWalkable = false;

	// 2. 정빈님의 스폰 코드에 맞춘 정확한 시작/끝 좌표!
	int32 SpawnerX = 0;
	int32 SpawnerY = 0;
	int32 NexusX = GridWidth - 1; 
	int32 NexusY = GridHeight - 1; 

	// 💡 경로를 담아올 빈 가방을 하나 임시로 만듭니다.
	TArray<FIntPoint> DummyPath; 
    
	// 가방(DummyPath)도 같이 넘겨줍니다!
	bool bIsPathClear = FindPath(SpawnerX, SpawnerY, NexusX, NexusY, DummyPath);

	// 3. 증거 인멸 (다시 걷기 가능 상태로 롤백)
	GridArray[Index].bIsWalkable = true;

	// 4. 판결: 길이 아예 막혀버렸다면?
	if (!bIsPathClear)
	{
		// 화면에 빨간색 경고 띄우고
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 몬스터가 지나갈 길은 남겨둬야 합니다!"));
       
		// 돈 안 깎고 함수 종료! (설치 취소)
		return; 
	}
	
	TSubclassOf<AActor> ClassToSpawn = SelectedData->TowerActorClass;
	if (!ClassToSpawn) return;

	AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		int32 TowerCost = 20;

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
	GridArray[Index].bIsWalkable = false; // 이제 진짜로 못 지나가는 길이 됨!

	// ========================================================
	// 💡 [추가 로직] 필드의 모든 몬스터에게 경로 재설정 명령 내리기
	// ========================================================

	// 3. 월드에 있는 모든 EnemyBase 클래스를 찾습니다.
	TArray<AActor*> FoundEnemies;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEnemyBase::StaticClass(), FoundEnemies);

	for (AActor* Actor : FoundEnemies)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
        
		// 죽지 않은 몬스터들만 골라서 새 지도를 쥐여줍니다.
		if (Enemy && !Enemy->IsDead())
		{
			// 몬스터가 가진 RecalculatePath() 함수 호출 (아래에서 만들 예정!)
			Enemy->RecalculatePath(); 
		}
	}
    
	UE_LOG(LogTemp, Warning, TEXT("타워 건설 완료: 모든 몬스터의 경로를 갱신합니다."));
}

bool AGridManager::bIsTileBuildable(int32 X, int32 Y) const
{
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return false;
	return GridArray[Index].TileType == ETileType::Empty;
}

int32 AGridManager::GetDistance(int32 NodeAX, int32 NodeAY, int32 NodeBX, int32 NodeBY)
{
	int32 X = FMath::Abs(NodeAX - NodeBX);
	int32 Y = FMath::Abs(NodeAY - NodeBY);

	return (X + Y) * 10;
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

bool AGridManager::FindPath(int32 StartX, int32 StartY, int32 EndX, int32 EndY, TArray<FIntPoint>& OutPath)
{
	if (!GridArray.IsValidIndex(GetIndex(StartX, StartY)) || !GridArray.IsValidIndex(GetIndex(EndX, EndY))) return false;

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
	return false;
}

