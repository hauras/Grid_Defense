
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EffectActor.generated.h"

class UNiagaraComponent;
class APoolManager;

UCLASS()
class GRID_DEFENSE_API AEffectActor : public AActor
{
	GENERATED_BODY()
	
public:	
	AEffectActor();

	UFUNCTION(BlueprintCallable, Category = "Effect")
	void PlayEffect(float LifeTime = 0.2f);
	
protected:
	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UNiagaraComponent> NiagaraComp;

	UPROPERTY()
	TObjectPtr<APoolManager> CachedPoolManager;

	FTimerHandle ReturnTimerHandle;

	UFUNCTION()
	void ReturnToManager();

};
