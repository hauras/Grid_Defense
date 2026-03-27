
#pragma once

#include "CoreMinimal.h"
#include "Tower/TowerBase.h"
#include "LightningTower.generated.h"

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API ALightningTower : public ATowerBase
{
	GENERATED_BODY()
public:
	ALightningTower();

protected:

	virtual void Fire() override;
	
	UPROPERTY(EditAnywhere, Category = "Tower")
	int32 MaxChainCount = 3;

	UPROPERTY(EditAnywhere, Category = "Tower")
	float ChainRange = 500.f;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<class AEffectActor> LightningEffectClass;

	void ExecuteChain(AActor* Target, int32 CurrentChainCount, TArray<AActor*> HitActors); 
};
