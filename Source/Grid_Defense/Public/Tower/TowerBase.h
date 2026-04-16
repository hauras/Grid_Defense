
#pragma once

#include "CoreMinimal.h"
#include "Data/TowerData.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h" 
#include "TowerBase.generated.h"

class AGridManager;

UENUM(BlueprintType)
enum class ETargetPriority  : uint8
{
	First UMETA(DisplayName = "First (Closest to Nexus"),
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

	UPROPERTY()
	AActor* CurrentTarget;

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

	
};
