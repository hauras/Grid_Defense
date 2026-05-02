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

// =====================================================================
// 유틸리티
// =====================================================================

FVector AGridManager::GetTileWorldPosition(int32 X, int32 Y) const
{
    int32 Index = GetIndex(X, Y);
    if (GridArray.IsValidIndex(Index))
    {
        return GridArray[Index].WorldPosition;
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
    return INVALID_FLOW_COST;
}

// [수정] const 추가
TArray<FIntPoint> AGridManager::GetWalkableNeighbors(int32 X, int32 Y) const
{
    TArray<FIntPoint> Neighbors;

    const FIntPoint Directions[4] = {
        FIntPoint(0, 1), FIntPoint(0, -1), FIntPoint(1, 0), FIntPoint(-1, 0)
    };

    for (const auto& Dir : Directions)
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

// [수정] bIsTileBuildable → IsTileBuildable
bool AGridManager::IsTileBuildable(int32 X, int32 Y) const
{
    int32 Index = GetIndex(X, Y);
    if (!GridArray.IsValidIndex(Index)) return false;
    return GridArray[Index].TileType == ETileType::Empty;
}

// =====================================================================
// Flow Field
// =====================================================================

void AGridManager::UpdateFlowField()
{
    // 1. 초기화
    for (int i = 0; i < GridArray.Num(); ++i)
    {
        GridArray[i].FlowCost = INVALID_FLOW_COST;
        GridArray[i].FlowDirection = FVector::ZeroVector;
    }

    TQueue<FIntPoint> Queue;
    const int32 NexusX = GridWidth - 1;
    const int32 NexusY = GridHeight - 1;
    const int32 NexusIndex = GetIndex(NexusX, NexusY);

    GridArray[NexusIndex].FlowCost = 0;
    Queue.Enqueue(FIntPoint(NexusX, NexusY));

    // 2. Integration Field (BFS)
    while (!Queue.IsEmpty())
    {
        FIntPoint CurrentNode;
        Queue.Dequeue(CurrentNode);

        int32 CurrentIndex = GetIndex(CurrentNode.X, CurrentNode.Y);
        int32 CurrentCost = GridArray[CurrentIndex].FlowCost;

        TArray<FIntPoint> Neighbors = GetWalkableNeighbors(CurrentNode.X, CurrentNode.Y);
        for (const auto& Neighbor : Neighbors)
        {
            int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
            if (GridArray[NeighborIndex].FlowCost > CurrentCost + 1)
            {
                GridArray[NeighborIndex].FlowCost = CurrentCost + 1;
                Queue.Enqueue(Neighbor);
            }
        }
    }

    // 3. Flow Direction 계산
    for (int i = 0; i < GridArray.Num(); ++i)
    {
        if (GridArray[i].FlowCost >= INVALID_FLOW_COST || GridArray[i].FlowCost == 0)
        {
            continue;
        }

        TArray<FIntPoint> Neighbors = GetWalkableNeighbors(GridArray[i].X, GridArray[i].Y);

        int32 BestCost = GridArray[i].FlowCost;
        FIntPoint BestNeighborPos = FIntPoint(GridArray[i].X, GridArray[i].Y);
        bool bFoundPath = false;

        for (const auto& Neighbor : Neighbors)
        {
            int32 NeighborIndex = GetIndex(Neighbor.X, Neighbor.Y);
            if (GridArray[NeighborIndex].FlowCost < BestCost)
            {
                BestCost = GridArray[NeighborIndex].FlowCost;
                BestNeighborPos = Neighbor;
                bFoundPath = true;
            }
        }

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

    if (GridArray.IsValidIndex(Index))
    {
        FVector BakedDir = GridArray[Index].FlowDirection;
        FVector CurrentTileCenter = GridArray[Index].WorldPosition;
        FVector TargetTileCenter = CurrentTileCenter + (BakedDir * TileSize);

        FVector CorrectedDir = TargetTileCenter - WorldLocation;
        CorrectedDir.Z = 0.f;
        return CorrectedDir.GetSafeNormal();
    }

    return FVector::ZeroVector;
}

// =====================================================================
// BeginPlay / 맵 생성
// =====================================================================

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

                for (const FTowerSaveData& SavedTower : LoadedGame->SavedTowers)
                {
                    AddTower(SavedTower.GridX, SavedTower.GridY, SavedTower.TowerData, true);
                }

                AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
                if (GM)
                {
                    GM->SetCurrentGold(LoadedGame->SavedGold);
                }

                return;
            }
        }
    }

    GenerateGrid();
}

// [수정] SpawnGameActors로 스포너·넥서스 스폰 분리 — GenerateGrid/LoadSavedGrid 중복 제거
void AGridManager::SpawnGameActors()
{
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
        GetWorld()->SpawnActor<ANexus>(NexusClass, NexusLoc, FRotator::ZeroRotator);
    }
}

void AGridManager::GenerateGrid()
{
    const int32 TotalTiles = GridWidth * GridHeight;
    const FVector ManagerLocation = GetActorLocation();

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

                if (X == 0 && Y == 0)                               Node.TileType = ETileType::Start;
                else if (X == GridWidth - 1 && Y == GridHeight - 1) Node.TileType = ETileType::End;
                else if (FMath::FRandRange(0.f, 100.f) < 15.f)      Node.TileType = ETileType::Rock;
                else                                                 Node.TileType = ETileType::Empty;

                Node.bIsWalkable = (Node.TileType != ETileType::Rock);
            }
        }

        UpdateFlowField();

        if (GridArray[GetIndex(0, 0)].FlowCost < INVALID_FLOW_COST)
        {
            bIsValidMap = true;
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
                case ETileType::Empty: FloorISM->AddInstance(TileTransform);    break;
                case ETileType::Rock:  ObstacleISM->AddInstance(TileTransform); break;
                case ETileType::Start: StartISM->AddInstance(TileTransform);    break;
                case ETileType::End:   EndISM->AddInstance(TileTransform);      break;
                default: break;
            }
        }
    }

    SpawnGameActors();
}

void AGridManager::LoadSavedGrid(const TArray<ETileType>& SavedLayout)
{
    if (SavedLayout.Num() == 0) return;

    const int32 TotalTiles = GridWidth * GridHeight;
    const FVector ManagerLocation = GetActorLocation();

    GridArray.Empty();
    GridArray.SetNum(TotalTiles);

    for (int32 i = 0; i < SavedLayout.Num(); ++i)
    {
        int32 X = i % GridWidth;
        int32 Y = i / GridWidth;

        FGridInfo& Node = GridArray[i];
        Node.X = X;
        Node.Y = Y;
        Node.WorldPosition = ManagerLocation + FVector(X * TileSize, Y * TileSize, 0.f);
        Node.TileType = SavedLayout[i];
        Node.bIsWalkable = (Node.TileType != ETileType::Rock && Node.TileType != ETileType::Tower);
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
                case ETileType::Empty:
                case ETileType::Tower: FloorISM->AddInstance(TileTransform);    break;
                case ETileType::Rock:  ObstacleISM->AddInstance(TileTransform); break;
                case ETileType::Start: StartISM->AddInstance(TileTransform);    break;
                case ETileType::End:   EndISM->AddInstance(TileTransform);      break;
                default: break;
            }
        }
    }

    UpdateFlowField();
    SpawnGameActors();
}

// =====================================================================
// 타워 건설
// =====================================================================

void AGridManager::AddTower(int32 X, int32 Y, UTowerData* SelectedData, bool bIsLoading)
{
    if (!SelectedData) return;

    int32 Index = GetIndex(X, Y);
    if (!GridArray.IsValidIndex(Index)) return;

    if (!bIsLoading)
    {
        if (!IsTileBuildable(X, Y))
        {
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 이미 장애물이나 타워가 있습니다."));
            return;
        }

        GridArray[Index].bIsWalkable = false;
        UpdateFlowField();

        const bool bIsPathClear = (GridArray[GetIndex(0, 0)].FlowCost < INVALID_FLOW_COST);

        if (!bIsPathClear)
        {
            GridArray[Index].bIsWalkable = true;
            UpdateFlowField(); // 롤백 복구
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("설치 불가: 드래곤의 길을 완전히 막을 수 없습니다!"));
            return;
        }

        AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
        if (!GM) return; 

        if (!GM->SpendGold(SelectedData->BuildCost))
        {
            GridArray[Index].bIsWalkable = true;
            UpdateFlowField(); // 롤백 복구
            if (GEngine)
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("잔액 부족! 타워를 건설할 수 없습니다."));
            return;
        }
    }

    // 타일 데이터 확정
    GridArray[Index].TileType = ETileType::Tower;
    GridArray[Index].bIsWalkable = false;

    // 액터 스폰
    if (TSubclassOf<AActor> ClassToSpawn = SelectedData->TowerActorClass)
    {
        FVector SpawnLocation = GridArray[Index].WorldPosition + FVector(0.f, 0.f, 50.f);

        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator, SpawnParams);
        if (ATowerBase* Tower = Cast<ATowerBase>(SpawnedActor))
        {
            Tower->InitTower(SelectedData, false);
            Tower->GridX = X;
            Tower->GridY = Y;
            GridArray[Index].OccupyingTower = Tower;
        }
    }

    if (!bIsLoading)
    {
        // 로딩 중이 아닐 때는 경로 재계산이 이미 위에서 완료됨
        UE_LOG(LogTemp, Warning, TEXT("[%d, %d] 타워 건설 완료 및 경로 업데이트."), X, Y);
    }
    else
    {
        // 로딩 중일 때는 모든 타워 스폰 후 BeginPlay에서 한 번에 UpdateFlowField 불필요
        // LoadSavedGrid에서 이미 호출됨
    }
}

void AGridManager::RemoveTower(int32 X, int32 Y)
{
    int32 Index = GetIndex(X, Y);
    if (!GridArray.IsValidIndex(Index)) return;

    ATowerBase* Tower = GridArray[Index].OccupyingTower;
    if (!Tower) return;

    AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!GM) return;

    UTowerData* TowerData = Tower->GetTowerData();
    if (!TowerData) return;

    int32 RefundAmount = TowerData->BuildCost * 0.5f;
    GM->AddGold(RefundAmount);

    GridArray[Index].TileType = ETileType::Empty;
    GridArray[Index].bIsWalkable = true;
    GridArray[Index].OccupyingTower = nullptr;

    UpdateFlowField();
    Tower->Destroy();
}


