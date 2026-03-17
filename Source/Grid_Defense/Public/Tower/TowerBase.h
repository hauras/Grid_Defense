
#pragma once

#include "CoreMinimal.h"
#include "Data/TowerData.h"
#include "GameFramework/Actor.h"
#include "TowerBase.generated.h"

UCLASS()
class GRID_DEFENSE_API ATowerBase : public AActor
{
	GENERATED_BODY()
	
public:	
	ATowerBase();

	void InitTower(UTowerData* TowerData, bool bIsPreview = false);

	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UDecalComponent> RangeDecal;

	UPROPERTY()
	TObjectPtr<UTowerData> MyData;

	bool bIsPreviewMode = false;

	UPROPERTY()
	AActor* CurrentTarget;

	FTimerHandle AttackTimerHandle;

	void FindTarget();

	void Fire();
};
