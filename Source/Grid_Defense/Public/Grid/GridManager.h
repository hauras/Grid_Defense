#pragma once
 
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "GridManager.generated.h"
 
class ANexus;
class AEnemySpawner;
class UTowerData;
class ATowerBase;

inline constexpr int32 INVALID_FLOW_COST = 99999;
 
UENUM(BlueprintType)
enum class ETileType : uint8
{
    Empty,
    Rock,
    Lake,
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
    int32 FlowCost;
    FVector FlowDirection;

    // [추가] 해당 타일을 점유중인 타워 포인터
    UPROPERTY()
    TObjectPtr<ATowerBase> OccupyingTower = nullptr;
 
    FGridInfo()
        : X(0)
        , Y(0)
        , WorldPosition(FVector::ZeroVector)
        , bIsWalkable(true)
        , TileType(ETileType::Empty)
        , FlowCost(INVALID_FLOW_COST)
        , FlowDirection(FVector::ZeroVector)
        , OccupyingTower(nullptr)
    {}
};
 
UCLASS()
class GRID_DEFENSE_API AGridManager : public AActor
{
    GENERATED_BODY()
    
public: 
    AGridManager();
 
    FORCEINLINE int32 GetIndex(int32 X, int32 Y) const { return (Y * GridWidth) + X; }
 
    void AddTower(int32 X, int32 Y, UTowerData* SelectedData, bool bIsLoading = false);

    // [수정] 좌표만 받으면 됨 — GridArray에서 타워 포인터를 직접 꺼냄
    void RemoveTower(int32 X, int32 Y);
 
    bool IsTileBuildable(int32 X, int32 Y) const;
 
    float GetTileSize() const { return TileSize; }
    int32 GetGridWidth() const { return GridWidth; }
    int32 GetGridHeight() const { return GridHeight; }
    FVector GetTileWorldPosition(int32 X, int32 Y) const;
 
    void UpdateFlowField();
    FVector GetFlowDirection(FVector WorldLocation) const;
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
 
    UPROPERTY()
    TObjectPtr<AEnemySpawner> ActiveSpawner;
 
private:
    TArray<FGridInfo> GridArray;
 
    void DrawDebugFlowField();
    TArray<FIntPoint> GetWalkableNeighbors(int32 X, int32 Y) const;
    void LoadSavedGrid(const TArray<ETileType>& SavedLayout);
    void SpawnGameActors();
 
    // [수정] TObjectPtr로 변경
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInstancedStaticMeshComponent> FloorISM;
 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInstancedStaticMeshComponent> ObstacleISM;
 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInstancedStaticMeshComponent> StartISM;
 
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UInstancedStaticMeshComponent> EndISM;
 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
    int32 GridWidth;
 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
    int32 GridHeight;
 
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Grid", meta = (AllowPrivateAccess = "true"))
    float TileSize;
};