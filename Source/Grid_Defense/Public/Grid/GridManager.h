
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
    
	// 💡 [새로 추가된 플로우 필드(BFS)용 데이터]
	int32 FlowCost;          // 넥서스까지 몇 칸 남았는지 (기본값: 무한대처럼 큰 수)
	FVector FlowDirection;   // 몬스터가 걸어갈 방향 (화살표)
    
	// 생성자도 업데이트 해줍니다!
	FGridInfo() : X(0), Y(0), WorldPosition(FVector::ZeroVector), bIsWalkable(true), FlowCost(99999), FlowDirection(FVector::ZeroVector) {}
};

// A* 알고리즘 추후 필요하면 사용
#if 0
struct FAStarNode
{
	int32 X,Y;
    
	int32 GCost; // 시작점에서의 거리
	int32 HCost; // 목적지까지의 예상거리
	int32 FCost; // G +H를 합한 값 예상거리 + 시작점에서거리
    
	FAStarNode* ParentNode;

	FAStarNode(int32 InX, int32 InY) : X(InX), Y(InY), GCost(0), HCost(0), FCost(0), ParentNode(nullptr) {}    
};

#endif


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

// 💡 A* 길찾기 함수는 이제 안 쓰므로 봉인! (나중에 포트폴리오 비교용)
#if 0
    bool FindPath(int32 StartX, int32 StartY, int32 EndX, int32 EndY, TArray<FIntPoint>& OutPath);
#endif
    
    float GetTileSize() const { return TileSize; }
    int32 GetGridWidth() const { return GridWidth; }
    int32 GetGridHeight() const { return GridHeight; }
    FVector GetTileWorldPosition(int32 X, int32 Y) const;

    // 💡 [플로우 필드 핵심 1] 맵 전체에 물을 뿌려서 거리를 계산하고 화살표를 그립니다.
    void UpdateFlowField();
    
    // 💡 [플로우 필드 핵심 2] 몬스터가 자기 위치를 주면, 어디로 걸어가야 할지 화살표(방향)를 반환합니다.
    FVector GetFlowDirection(FVector WorldLocation) const;

    // 💡 [유틸리티] 몬스터의 실제 월드 위치(FVector)를 그리드 타일 좌표(X, Y)로 변환해 줍니다.
    FIntPoint GetGridPointFromWorld(FVector WorldLocation) const;

	UFUNCTION(BlueprintCallable, Category = "FlowField")
	int32 GetFlowCost(FVector WorldLocation) const;
protected:
    virtual void BeginPlay() override;
    void GenerateGrid();

    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AEnemySpawner> SpawnerClass;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<ANexus> NexusClass;
    
    AEnemySpawner* ActiveSpawner;

private:
	
    TArray<FGridInfo> GridArray;


	void DrawDebugFlowField();
	
    TArray<FIntPoint> GetWalkableNeighbors(int32 X, int32 Y);

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