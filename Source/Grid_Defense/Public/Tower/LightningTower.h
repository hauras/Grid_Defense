#pragma once

#include "CoreMinimal.h"
#include "Tower/TowerBase.h"
#include "LightningTower.generated.h"

UCLASS()
class GRID_DEFENSE_API ALightningTower : public ATowerBase
{
	GENERATED_BODY()
public:
	ALightningTower();

protected:
	virtual void Fire() override;

	UPROPERTY(EditAnywhere, Category = "Effects")
	TSubclassOf<class AEffectActor> LightningEffectClass;

	void ExecuteChain(AActor* Target, int32 CurrentChainCount, TArray<AActor*> HitActors);

private:
	// 진행 중인 체인 타이머 목록 — 스턴/사망 시 전체 취소용
	TArray<FTimerHandle> ChainTimerHandles;
};