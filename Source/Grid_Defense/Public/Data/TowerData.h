#pragma once
 
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "TowerData.generated.h"
 
class UNiagaraSystem;
 
UENUM(BlueprintType)
enum class ETowerType : uint8
{
    SingleTarget, 
    AoE,         
    Chain, 
};
 
UCLASS()
class GRID_DEFENSE_API UTowerData : public UPrimaryDataAsset
{
    GENERATED_BODY()
public:
 
    UPROPERTY(EditAnywhere, Category = "Info")
    ETowerType TowerType = ETowerType::SingleTarget;
 
    UPROPERTY(EditAnywhere, Category = "Info")
    float Damage = 10.f;
 
    UPROPERTY(EditAnywhere, Category = "Info")
    float AttackInterval = 1.0f;
 
    // 단위: cm (예: 1000 = 10m)
    UPROPERTY(EditAnywhere, Category = "Info", meta = (Units = "cm", ClampMin = "0"))
    float AttackRange = 1000.f;
 
    UPROPERTY(EditAnywhere, Category = "Info")
    TObjectPtr<UMaterialInterface> RangeDecalMaterial;
    
    // AoE 전용
    UPROPERTY(EditAnywhere, Category = "Info", meta = (EditCondition = "TowerType == ETowerType::AoE", EditConditionHides))
    float SplashRadius = 300.f;
 
    // Chain 전용
    UPROPERTY(EditAnywhere, Category = "Info", meta = (EditCondition = "TowerType == ETowerType::Chain", EditConditionHides))
    int32 ChainCount = 3;
 
    // Chain 전용 - 단위: cm
    UPROPERTY(EditAnywhere, Category = "Info", meta = (EditCondition = "TowerType == ETowerType::Chain", EditConditionHides, Units = "cm", ClampMin = "0"))
    float ChainRange = 2000.f;
    
    UPROPERTY(EditAnywhere, Category = "Info")
    TObjectPtr<UStaticMesh> PreviewMesh;
 
    UPROPERTY(EditAnywhere, Category = "Visual")
    TObjectPtr<UTexture2D> TowerIcon;
    
    UPROPERTY(EditAnywhere, Category = "Economy")
    int32 BuildCost = 100;
 
    UPROPERTY(EditAnywhere, Category = "Visual")
    TSubclassOf<AActor> TowerActorClass;
 
    UPROPERTY(EditAnywhere, Category = "Info")
    FName TowerName;
 
    UPROPERTY(EditAnywhere, Category = "Info", meta = (MultiLine = true))
    FText Description;
 
    // 데칼 반지름 배율 (AttackRange * DecalMultiplier = 실제 표시 반지름)
    UPROPERTY(EditAnywhere, Category = "Visual")
    float DecalMultiplier = 1.0f;
}; 