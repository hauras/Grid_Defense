
#pragma once

#include "CoreMinimal.h"
#include "Tower/TowerBase.h"
#include "IceTower.generated.h"

class UNiagaraComponent;
class USphereComponent;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API AIceTower : public ATowerBase
{
	GENERATED_BODY()
public:

	AIceTower();

protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<USphereComponent> IceSphere;

	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UNiagaraComponent> MistEffect;

	void PulseAura();
	FTimerHandle AuraPulseTimer;
	
	virtual void Fire() override;
	virtual void InitTower(UTowerData* TowerData, bool bIsPreview = false) override;
	
};
