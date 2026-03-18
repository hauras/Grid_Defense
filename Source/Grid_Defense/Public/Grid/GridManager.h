
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GridManager.generated.h"

class AEnemySpawner;
class UTowerData;

UENUM(BlueprintType)
enum class ETileType : uint8
{
	Empty,
	Rock,// 장애물 1
	Lake, // 장애물 2 호수
	Tower,
	Start,
	End
};

USTRUCT(BlueprintType)
struct FGridInfo
{
	GENERATED_BODY()

	int32 X;
	int32 Y;
	FVector WorldPosition;
	bool bIsWalkable;
	ETileType TileType;
	
	FGridInfo() : X(0), Y(0), WorldPosition(FVector::ZeroVector), bIsWalkable(true) {}
};

UCLASS()
class GRID_DEFENSE_API AGridManager : public AActor
{
	GENERATED_BODY()
	
public:	
	AGridManager();

	FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return (Y * GridWidth) + X; }

	void AddTower(int32 X, int32 Y, UTowerData* SelectedData);

	bool bIsTileBuildable(int32 X, int32 Y) const;
	
protected:
	virtual void BeginPlay() override;
	
	void GenerateGrid();

	UPROPERTY(EditAnywhere, Category = " Grid")
	TSubclassOf<AEnemySpawner> SpawnerClass;

	AEnemySpawner* ActiveSpawner;
public:

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* FloorISM;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* ObstacleISM;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* StartISM;

	UPROPERTY(VisibleAnywhere)
	UInstancedStaticMeshComponent* EndISM;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 GridWidth;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	int32 GridHeight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid")
	float TileSize;

	UPROPERTY(EditAnywhere, Category = "Tower")
	TSubclassOf<AActor> TowerClass;
	
private:
	TArray<FGridInfo> GridArray;

};
