#pragma once

#include "CoreMinimal.h"
#include "Tower/TowerBase.h"
#include "IceTower.generated.h"

class UNiagaraComponent;

UCLASS()
class GRID_DEFENSE_API AIceTower : public ATowerBase
{
	GENERATED_BODY()
public:
	AIceTower();

protected:
	UPROPERTY(VisibleAnywhere, Category = "Effect")
	TObjectPtr<UNiagaraComponent> MistEffect;

	// 슬로우 지속 시간 (초)
	UPROPERTY(EditAnywhere, Category = "Tower")
	float SlowDuration = 1.0f;

	virtual void InitTower(UTowerData* TowerData, bool bIsPreview = false) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	void PulseAura();
	FTimerHandle AuraPulseTimer;
};