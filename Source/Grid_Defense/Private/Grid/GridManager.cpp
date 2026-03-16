#include "Grid/GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h" 
#include "Tower/TowerBase.h"

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

	TSubclassOf<AActor> ClassToSpawn = SelectedData->TowerActorClass;
	if (!ClassToSpawn) return;

	// 💡 수정: 실제 타워 생성 시에도 Z축을 프리뷰와 동일하게 위로 올려줍니다.
	FVector SpawnLocation = GridArray[Index].WorldPosition + FVector(0.f, 0.f, 50.f);
    
	AActor* SpawnedActor = GetWorld()->SpawnActor<AActor>(ClassToSpawn, SpawnLocation, FRotator::ZeroRotator);

	if (ATowerBase* Tower = Cast<ATowerBase>(SpawnedActor))
	{
		Tower->InitTower(SelectedData, false); // 실제 타워 모드
	}

	GridArray[Index].TileType = ETileType::Tower;
	GridArray[Index].bIsWalkable = false;
}

bool AGridManager::bIsTileBuildable(int32 X, int32 Y) const
{
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return false;
	return GridArray[Index].TileType == ETileType::Empty;
}