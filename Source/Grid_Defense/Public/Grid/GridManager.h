
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


UCLASS()
class GRID_DEFENSE_API AGridManager : public AActor
{
    GENERATED_BODY()
    
public: 
    AGridManager();

    // =====================================
    FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return (Y * GridWidth) + X; }
	void AddTower(int32 X, int32 Y, UTowerData* SelectedData, bool bIsLoading = false);
	
    bool bIsTileBuildable(int32 X, int32 Y) const;
	
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

	const TArray<FGridInfo>& GetGridArray() const { return GridArray; }	
protected:
    virtual void BeginPlay() override;
    void GenerateGrid();

    UPROPERTY(EditAnywhere, Category = "Grid")
    TSubclassOf<AEnemySpawner> SpawnerClass;

    UPROPERTY(EditAnywhere, Category = "Spawning")
    TSubclassOf<ANexus> NexusClass;
    
    AEnemySpawner* ActiveSpawner;

	void LoadSavedGrid(const TArray<ETileType>& SavedLayout);
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