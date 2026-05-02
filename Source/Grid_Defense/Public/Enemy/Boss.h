#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "Boss.generated.h"

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
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathCooldown = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathRange = 600.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathWidth = 250.f;

	// [수정] 스턴 지속 시간 UPROPERTY로 노출
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathStunDuration = 3.0f;

	// [수정] 타워 미발견 시 재검사 간격 UPROPERTY로 노출
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	float BreathCheckInterval = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
	TObjectPtr<UAnimMontage> BreathMontage;

private:
	FTimerHandle BreathCooldownTimer;

	void StartFireBreath();

	// [수정] 중복 BoxTrace 로직 분리
	TArray<FHitResult> PerformBreathTrace();
};