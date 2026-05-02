#pragma once

#include "CoreMinimal.h"
#include "Data/TowerData.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h" 
#include "State/GridGameState.h"
#include "TowerBase.generated.h"

class AGridManager;
class UWidgetComponent;

UENUM(BlueprintType)
enum class ETargetPriority : uint8
{
    First UMETA(DisplayName = "First (Closest to Nexus)"),
    Strong,
    Weak,
};

class APoolManager;
class AProjectileBase;

UCLASS()
class GRID_DEFENSE_API ATowerBase : public AActor
{
    GENERATED_BODY()
    
public: 
    ATowerBase();

    virtual void InitTower(UTowerData* TowerData, bool bIsPreview = false);

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
    FGameplayTagContainer TowerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FGameplayTag TowerDamageTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
    FGameplayTagContainer StateTag;

    void ApplyStun(float StunDuration);
    
    UTowerData* GetTowerData() const { return MyData; }
    bool IsPreview() const { return bIsPreviewMode; }

    UPROPERTY(VisibleAnywhere, Category = "Grid")
    int32 GridX;

    UPROPERTY(VisibleAnywhere, Category = "Grid")
    int32 GridY;

    UFUNCTION()
    void ReceiveBuffBroadcast(const FCardData& CardInfo);

protected:
    virtual void BeginPlay() override;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USceneComponent> Root;
    
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UStaticMeshComponent> MeshComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UDecalComponent> RangeDecal;

    UPROPERTY(EditAnywhere, Category = "Effects")
    TObjectPtr<USoundBase> AttackSound;
    
    UPROPERTY()
    TObjectPtr<UTowerData> MyData;

    UPROPERTY(EditDefaultsOnly, Category = "Tower")
    TSubclassOf<AProjectileBase> ProjectileClass;

    bool bIsPreviewMode = false;

    // [수정] TObjectPtr로 변경
    UPROPERTY()
    TObjectPtr<AActor> CurrentTarget;

    FTimerHandle AttackTimerHandle;
    FTimerHandle StunTimerHandle;

    void EndStun();
    void FindTarget();

    virtual void Fire();

    UPROPERTY()
    TObjectPtr<APoolManager> CachedPoolManager;

    UPROPERTY()
    TObjectPtr<AGridManager> CachedGridManager;

    UPROPERTY(EditAnywhere, Category = "Tower")
    ETargetPriority TargetPriority = ETargetPriority::First;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tower")
    TObjectPtr<UWidgetComponent> StunWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower")
    float CurrentDamageMultiplier = 1.0f;

    // [수정] CurrentRangeMultiplier 제거 — 실제 사용처 없음

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower")
    float CurrentAttackSpeedMultiplier = 1.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower")
    int32 CurrentChainBonus = 0;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tower")
    float CurrentSplashRadiusBonus = 0.0f;

    void UpdateAttackTimer();
};