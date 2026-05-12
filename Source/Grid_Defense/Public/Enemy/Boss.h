#pragma once

#include "CoreMinimal.h"
#include "Enemy/EnemyBase.h"
#include "Boss.generated.h"

//class UBossGimmickWidget;

UCLASS()
class GRID_DEFENSE_API ABoss : public AEnemyBase
{
    GENERATED_BODY()
public:
    ABoss();

    UFUNCTION(BlueprintCallable, Category = "Boss|Pattern")
    void FireBreath();

    void EndFireBreath();

    // 기믹 성공/실패 시 GridController에서 호출
    void OnGimmickSuccess();
    void OnGimmickFail();

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator, AActor* DamageCauser) override;

    // --- 브레스 패턴 ---
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern")
    float BreathCooldown = 10.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern")
    float BreathRange = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern")
    float BreathWidth = 250.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern")
    float BreathStunDuration = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Pattern")
    float BreathCheckInterval = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Boss|Pattern")
    TObjectPtr<UAnimMontage> BreathMontage;

    // --- 페이즈 전환 ---
    // HP 비율 (0.5 = 50%)
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    float PhaseHPThreshold = 0.5f;

    // 기절 지속 시간
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    float StunDuration = 5.0f;

    // 분노 시 이동속도 배율
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    float RageSpeedMultiplier = 1.5f;

    // 분노 시 데미지 배율
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    float RageDamageMultiplier = 1.5f;

    // 기믹 위젯 클래스
   /* UPROPERTY(EditDefaultsOnly, Category = "Boss|Phase")
    TSubclassOf<UBossGimmickWidget> GimmickWidgetClass;*/

private:
    FTimerHandle BreathCooldownTimer;
    FTimerHandle StunTimerHandle;

    void StartFireBreath();
    void PerformBreathTrace(TArray<FHitResult>& OutHitResults);

    // 페이즈 진입
    void EnterPhaseTwo();

    // 기절 종료
    void EndStun();

    // 런타임 플래그
    bool bPhaseTriggered = false;
    bool bIsStunned = false;
    bool bIsEnraged = false;

    // 기믹 위젯 캐시
    /*UPROPERTY()
    TObjectPtr<UBossGimmickWidget> CachedGimmickWidget;*/
};