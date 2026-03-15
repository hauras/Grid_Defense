#include "Grid/GridManager.h"
#include "Components/InstancedStaticMeshComponent.h"

AGridManager::AGridManager()
{
	PrimaryActorTick.bCanEverTick = false;

	FloorISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("FloorISM"));
	SetRootComponent(FloorISM); 

	// 나머지 ISM들도 생성 후 루트에 붙여줍니다.
	ObstacleISM = CreateDefaultSubobject<UInstancedStaticMeshComponent>(TEXT("ObstacleISM"));
	ObstacleISM->SetupAttachment(RootComponent); // 루트(FloorISM)에 딱 붙임

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

	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		for (int32 X = 0; X < GridWidth; ++X)
		{
			int32 Index = GetIndex(X, Y);
			FGridInfo& Node = GridArray[Index];

			Node.X = X;
			Node.Y = Y;
			Node.WorldPosition = FVector(X * TileSize, Y * TileSize, 0.f);

			// 1. 단순한 타입 결정 로직
			if (X == 0 && Y == 0) 
				Node.TileType = ETileType::Start;
			else if (X == GridWidth - 1 && Y == GridHeight - 1)
				Node.TileType = ETileType::End;
			else if (FMath::FRandRange(0.f, 100.f) < 15.f) // 15% 확률 장애물
				Node.TileType = ETileType::Rock;
			else
				Node.TileType = ETileType::Empty;

			// 2. 이동 가능 여부 (장애물만 아니면 다 갈 수 있음)
			Node.bIsWalkable = (Node.TileType != ETileType::Rock);

			// 3. 인스턴스 렌더링
			FTransform TileTransform(Node.WorldPosition);
			switch (Node.TileType)
			{
			case ETileType::Empty:    FloorISM->AddInstance(TileTransform); break;
			case ETileType::Rock: ObstacleISM->AddInstance(TileTransform); break;
			case ETileType::Start:    StartISM->AddInstance(TileTransform); break;
			case ETileType::End:      EndISM->AddInstance(TileTransform); break;
			}
		}
	}
}