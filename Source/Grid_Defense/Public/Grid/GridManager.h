
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GridManager.generated.h"

class ANexus;
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


struct FAStarNode
{
	int32 X,Y;
	
	int32 GCost; // 시작점에서의 거리
	int32 HCost; // 목적지까지의 예상거리
	int32 FCost; // G +H를 합한 값 예상거리 + 시작점에서거리
	
	FAStarNode* ParentNode;

	FAStarNode(int32 InX, int32 InY) : X(InX), Y(InY), GCost(0), HCost(0), FCost(0), ParentNode(nullptr) {}	
	
};
UCLASS()
class GRID_DEFENSE_API AGridManager : public AActor
{
    GENERATED_BODY()
    
public: 
    AGridManager();

 
    // =====================================
    FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return (Y * GridWidth) + X; }
    void AddTower(int32 X, int32 Y, UTowerData* SelectedData);
    bool bIsTileBuildable(int32 X, int32 Y) const;
    bool FindPath(int32 StartX, int32 StartY, int32 EndX, int32 EndY, TArray<FIntPoint>& OutPath);
	
	float GetTileSize() const { return TileSize; }
    int32 GetGridWidth() const { return GridWidth; }
    int32 GetGridHeight() const { return GridHeight; }
    FVector GetTileWorldPosition(int32 X, int32 Y) const;

protected:
    virtual void BeginPlay() override;
    void GenerateGrid();

    // =====================================
    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AEnemySpawner> SpawnerClass;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<ANexus> NexusClass;
    
    AEnemySpawner* ActiveSpawner;

private:
    // =====================================
    // =====================================
    TArray<FGridInfo> GridArray;

    // A* 내부용 연산 함수들도 여기로 숨김!
    int32 GetDistance(int32 NodeAX, int32 NodeAY, int32 NodeBX, int32 NodeBY);
    TArray<FIntPoint> GetWalkableNeighbors(int32 X, int32 Y);

    // 컴포넌트 숨기기 (AllowPrivateAccess 덕분에 블루프린트나 에디터에선 여전히 보임!)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UInstancedStaticMeshComponent* FloorISM;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UInstancedStaticMeshComponent* ObstacleISM;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UInstancedStaticMeshComponent* StartISM;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    UInstancedStaticMeshComponent* EndISM;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
    int32 GridWidth;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
    int32 GridHeight;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
    float TileSize;

    UPROPERTY(EditAnywhere, Category = "Tower", meta = (AllowPrivateAccess = "true"))
    TSubclassOf<AActor> TowerClass;
};
