
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
	
	UPROPERTY(EditAnywhere, Category = "Effect")
	TObjectPtr<UNiagaraComponent> MistEffect;

	void PulseAura();
	FTimerHandle AuraPulseTimer;
	
	virtual void InitTower(UTowerData* TowerData, bool bIsPreview = false) override;
	
};
