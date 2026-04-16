
#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "Boss.generated.h"

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API ABoss : public AEnemyBase
{
	GENERATED_BODY()
public:
	ABoss();

	UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
	void FireBreath();

	void EndFireBreath();
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathCooldown = 10.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathWidth = 250.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TObjectPtr<UAnimMontage> BreathMontage;

private:
	FTimerHandle BreathCooldownTimer;

	void StartFireBreath();;
};
