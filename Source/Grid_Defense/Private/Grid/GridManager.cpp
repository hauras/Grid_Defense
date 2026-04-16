#include "Grid/GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h" 
#include "Tower/TowerBase.h"
#include "Enemy/EnemySpawner.h"
#include "Enemy/EnemyBase.h"
#include "GameMode/GridGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Nexus/Nexus.h"
#include "Save/GridSaveGame.h"

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
		// [기존 코드] 고정된 나침반 방향 반환 (코너를 돌 때마다 오차 누적됨)
		// return GridArray[Index].FlowDirection;

		// 💡 [수정된 코드] 다음 타일의 중앙으로 끌어당기는 '자석' 조향 방식
       
		FVector BakedDir = GridArray[Index].FlowDirection;     // 원래 가야 할 방향
		FVector CurrentTileCenter = GridArray[Index].WorldPosition; // 현재 타일의 정중앙 좌표

		// 목적지 1: 우리가 도달해야 할 '다음 타일의 정중앙' 좌표 계산
		FVector TargetTileCenter = CurrentTileCenter + (BakedDir * TileSize);

		// 목적지 2: 몬스터의 '현재 위치'에서 '다음 타일 중앙'을 바라보는 벡터 계산!
		FVector CorrectedDir = TargetTileCenter - WorldLocation;
		CorrectedDir.Z = 0.f; // 땅바닥에 붙어있도록 높이 무시

		// 정규화(길이를 1로 만듦)해서 반환
		return CorrectedDir.GetSafeNormal();
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
    
	if (AGameModeBase* GameMode = GetWorld()->GetAuthGameMode())
	{
		FString Options = UGameplayStatics::ParseOption(GameMode->OptionsString, TEXT("LoadGame"));
        
		if (Options == TEXT("True"))
		{
			UGridSaveGame* LoadedGame = Cast<UGridSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("Slot1"), 0));
			if (LoadedGame)
			{
				LoadSavedGrid(LoadedGame->SavedMapLayout);

				// 그 후 타워를 복구합니다.
				for (const FTowerSaveData& SavedTower : LoadedGame->SavedTowers)
				{
					AddTower(SavedTower.GridX, SavedTower.GridY, SavedTower.TowerData, true);
				}

				AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
				if (GM)
				{
					GM->SetCurrentGold(LoadedGame->SavedGold); // Setter 사용! (UI 갱신까지 됨)
					//GM->SetCurrentLife(LoadedGame->SavedLife); // Setter 사용! (UI 갱신까지 됨)
				}
				
				return; 
			}
		}
	}

	GenerateGrid(); 
}

void AGridManager::GenerateGrid()
{
    int32 TotalTiles = GridWidth * GridHeight;
    FVector ManagerLocation = GetActorLocation();

    bool bIsValidMap = false;
    int32 SafetyCounter = 0;

    while (!bIsValidMap && SafetyCounter < 100)
    {
        SafetyCounter++;

        GridArray.Empty();
        GridArray.SetNum(TotalTiles);

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

        UpdateFlowField();

        if (GridArray[GetIndex(0, 0)].FlowCost < 99999)
        {
            bIsValidMap = true; // 합격! 루프를 탈출합니다.
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("길이 막힌 맵 생성됨! 재시도 중... (시도 횟수: %d)"), SafetyCounter);
        }
    }

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
    
}

void AGridManager::AddTower(int32 X, int32 Y, UTowerData* SelectedData, bool bIsLoading)
{
	if (!SelectedData) return;
    
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return;

	if (!bIsLoading)
	{
		if (!bIsTileBuildable(X, Y))
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 이미 장애물이나 타워가 있습니다."));
			return;
		}

		// --- 길 막기 시뮬레이션 시작 ---
		GridArray[Index].bIsWalkable = false; // 임시로 막음
		UpdateFlowField(); // 경로 재계산

		int32 SpawnerIndex = GetIndex(0, 0);
		bool bIsPathClear = (GridArray[SpawnerIndex].FlowCost < 99999);

		if (!bIsPathClear)
		{
			GridArray[Index].bIsWalkable = true; // 다시 길 열어주기
			UpdateFlowField(); // 필드 복구
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 드래곤의 길을 완전히 막을 수 없습니다!"));
			return;
		}
		// --- 길 막기 시뮬레이션 끝 ---

		// 골드 소모 체크
		AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
		if (GM)
		{
			if (!GM->SpendGold(SelectedData->BuildCost))
			{
				// 돈이 부족하면 시뮬레이션으로 막았던 길을 다시 열어주고 취소
				GridArray[Index].bIsWalkable = true;
				UpdateFlowField();
				GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("잔액 부족! 타워를 건설할 수 없습니다."));
				return;
			}
		}
	}

	// 2. [실제 데이터 확정] (공통 로직)
	GridArray[Index].TileType = ETileType::Tower;
	GridArray[Index].bIsWalkable = false;

	// 3. [액터 스폰]
	TSubclassOf<AActor> ClassToSpawn = SelectedData->TowerActorClass;
	if (ClassToSpawn)
	{
		FVector SpawnLocation = GridArray[Index].WorldPosition + FVector(0.f, 0.f, 50.f);
		
		// 소환 시 충돌로 인해 실패하지 않도록 AlwaysSpawn 설정
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams);

		if (ATowerBase* Tower = Cast<ATowerBase>(SpawnedActor))
		{
			Tower->InitTower(SelectedData, false); // 실제 타워로 작동

			// 세이브를 위해 좌표 저장
			Tower->GridX = X;
			Tower->GridY = Y;
		}
	}
	
	if (!bIsLoading)
	{
		UpdateFlowField();
		UE_LOG(LogTemp, Warning, TEXT("[%d, %d] 타워 건설 완료 및 경로 업데이트."), X, Y);
	}
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

void AGridManager::LoadSavedGrid(const TArray<ETileType>& SavedLayout)
{
    // 1. 저장된 데이터가 없으면 탈출
    if (SavedLayout.Num() == 0) return;

    int32 TotalTiles = GridWidth * GridHeight;
    FVector ManagerLocation = GetActorLocation();

    // 2. 맵 배열 초기화
    GridArray.Empty();
    GridArray.SetNum(TotalTiles);

    // 3. 저장된 타일 타입(바위, 길 등)을 그대로 복구
    for (int32 i = 0; i < SavedLayout.Num(); ++i)
    {
        int32 X = i % GridWidth;
        int32 Y = i / GridWidth;

        FGridInfo& Node = GridArray[i];
        Node.X = X;
        Node.Y = Y;
        Node.WorldPosition = ManagerLocation + FVector(X * TileSize, Y * TileSize, 0.f);
        
        // 🌟 저장된 타입을 그대로 꽂아 넣습니다!
        Node.TileType = SavedLayout[i];

        // 바위거나 타워가 있던 자리는 못 걷게 막아둡니다.
        Node.bIsWalkable = (Node.TileType != ETileType::Rock && Node.TileType != ETileType::Tower);
    }

    // 4. 화면에 타일 비주얼(ISM) 다시 그리기
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
                case ETileType::Empty:    
                case ETileType::Tower:    // 🌟 타워가 올라갈 자리도 일단 빈 땅(Floor)으로 깔아줍니다!
                    FloorISM->AddInstance(TileTransform); 
                    break;
                case ETileType::Rock:     
                    ObstacleISM->AddInstance(TileTransform); 
                    break;
                case ETileType::Start:    
                    StartISM->AddInstance(TileTransform); 
                    break;
                case ETileType::End:      
                    EndISM->AddInstance(TileTransform); 
                    break;
            }
        }
    }

    // 5. 복구된 맵을 바탕으로 경로(FlowField) 업데이트
    UpdateFlowField();

    // 6. 스포너와 넥서스 다시 생성하기
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
}