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

UENUM(BlueprintType)
enum class EStatusType : uint8
{
	None,
	Burn,
	Slow,
	Stun
};

USTRUCT(BlueprintType)
struct FStatus
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EStatusType Status = EStatusType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float EffectValue = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Duration = 0.f;
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
	
	UPROPERTY(EditAnywhere, Category = "Info", meta = (EditCondition = "TowerType == ETowerType::AoE", EditConditionHides))
	float SplashRadius = 300.f;

	UPROPERTY(EditAnywhere, Category = "Visual") // 카테고리 분리 추천
	TObjectPtr<UNiagaraSystem> ExplosionEffect;
	
	UPROPERTY(EditAnywhere, Category = "Visual")
	TObjectPtr<UStaticMesh> PreviewMesh;

	UPROPERTY(EditAnywhere, Category = "Status")
	FStatus StatusEffect;

	// 건설 비용도 데이터에 있으면 좋겠죠?
	UPROPERTY(EditAnywhere, Category = "Economy")
	int32 BuildCost = 100;

	UPROPERTY(EditAnywhere, Category = "Visual")
	TSubclassOf<AActor> TowerActorClass;

	UPROPERTY(EditAnywhere, Category = "Info")
	FText TowerName;

	UPROPERTY(EditAnywhere, Category = "Info", meta = (MultiLine = true))
	FText Description;
};