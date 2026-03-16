#include "Grid/GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Engine/World.h" //  타워 스폰을 위해 추가

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

void AGridManager::AddTower(int32 X, int32 Y)
{
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return;

	if (!bIsTileBuildable(X, Y))
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT(" 건설 불가: 장애물/타워가 있습니다!"));
		return;
	}
	if (!TowerClass)
	{
		GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT(" 에러: TowerClass 블루프린트가 할당되지 않았습니다!"));
		return;
	}

	// 타워 스폰
	FVector SpawnLocation = GridArray[Index].WorldPosition;
	GetWorld()->SpawnActor<AActor>(TowerClass, SpawnLocation, FRotator::ZeroRotator);

	GridArray[Index].TileType = ETileType::Tower;
	GridArray[Index].bIsWalkable = false;
	GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT(" 타워 건설 완료!"));
	
}

bool AGridManager::bIsTileBuildable(int32 X, int32 Y) const
{
	int32 Index = GetIndex(X, Y);
	if (!GridArray.IsValidIndex(Index)) return false;

	return GridArray[Index].TileType == ETileType::Empty;
	
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

	// 💡 핵심 수정: 매니저 액터의 현재 월드 위치를 가져옵니다.
	FVector ManagerLocation = GetActorLocation();

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			int32 Index = GetIndex(X, Y);
			FGridInfo& Node = GridArray[Index];

			Node.X = X;
			Node.Y = Y;
          
			// 💡 수정: 매니저의 위치(ManagerLocation)를 더해줍니다. 
			// 이렇게 해야 나중에 AddTower에서 이 좌표를 그대로 써도 정확한 위치에 소환됩니다.
			Node.WorldPosition = ManagerLocation + FVector(X * TileSize, Y * TileSize, 0.f);

			// 1. 타입 결정 로직 (동일)
			if (X == 0 && Y == 0) 
				Node.TileType = ETileType::Start;
			else if (X == GridWidth - 1 && Y == GridHeight - 1)
				Node.TileType = ETileType::End;
			else if (FMath::FRandRange(0.f, 100.f) < 15.f)
				Node.TileType = ETileType::Rock;
			else
				Node.TileType = ETileType::Empty;

			Node.bIsWalkable = (Node.TileType != ETileType::Rock);

			// 💡 3. 인스턴스 렌더링 수정
			// ISM은 기본적으로 액터의 로컬 좌표를 기준으로 인스턴스를 추가하므로, 
			// 여기서는 ManagerLocation이 더해지지 않은 '상대 좌표'가 필요합니다.
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