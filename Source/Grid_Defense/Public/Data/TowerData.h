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

	UPROPERTY(EditAnywhere, Category = "Info")
	float AttackRange = 1000.f;

	UPROPERTY(EditAnywhere, Category = "Info")
	TObjectPtr<UMaterialInterface> RangeDecalMaterial;
	
	UPROPERTY(EditAnywhere, Category = "Info", meta = (EditCondition = "TowerType == ETowerType::AoE", EditConditionHides))
	float SplashRadius = 300.f;
	
	UPROPERTY(EditAnywhere, Category = "Info")
	TObjectPtr<UStaticMesh> PreviewMesh;
	
	// 건설 비용도 데이터에 있으면 좋겠죠?
	UPROPERTY(EditAnywhere, Category = "Economy")
	int32 BuildCost = 100;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TSubclassOf<AActor> TowerActorClass;

	UPROPERTY(EditAnywhere, Category = "Info")
	FText TowerName;

	UPROPERTY(EditAnywhere, Category = "Info", meta = (MultiLine = true))
	FText Description;

	UPROPERTY(EditAnywhere, Category = "Visual")
	float DecalMultiplier = 1.0f;
};