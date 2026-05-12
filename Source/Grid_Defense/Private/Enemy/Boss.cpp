#include "Enemy/Boss.h"

#include "GridGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tower/TowerBase.h"

ABoss::ABoss()
{
}

// =====================================================================
// 초기화
// =====================================================================

void ABoss::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, BreathCooldown, false);
}

void ABoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(BreathCooldownTimer);
    GetWorldTimerManager().ClearTimer(StunTimerHandle);
}

// =====================================================================
// 데미지 처리 / 페이즈 전환
// =====================================================================

float ABoss::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    float Damage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

    if (!bPhaseTriggered && CurrentHP <= MaxHP * PhaseHPThreshold)
    {
        EnterPhaseTwo();
    }

    return Damage;
}

void ABoss::EnterPhaseTwo()
{
    bPhaseTriggered = true;
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.3f);

    // 위젯 추후 추가
}

// =====================================================================
// 기믹 결과 처리
// =====================================================================

void ABoss::OnGimmickSuccess()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
    GameplayTags.AddTag(FGridGameplayTags::Get().State_Stun);
    GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ABoss::EndStun, StunDuration, false);
}

void ABoss::OnGimmickFail()
{
    UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);
    bIsEnraged = true;

    // 이동속도 증가
    GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * RageSpeedMultiplier;

    // 브레스 쿨다운 감소 (절반으로)
    BreathCooldown *= 0.5f;
}

void ABoss::EndStun()
{
    GameplayTags.RemoveTag(FGridGameplayTags::Get().State_Stun);

    if (bIsEnraged)
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * RageSpeedMultiplier;
    else
        GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
}

// =====================================================================
// 브레스 패턴
// =====================================================================

void ABoss::PerformBreathTrace(TArray<FHitResult>& OutHitResults)
{
    FVector BoxHalfSize = FVector(BreathRange * 0.5f, BreathWidth, 100.f);
    FVector TraceCenter = GetActorLocation() + (GetActorForwardVector() * BoxHalfSize.X);
    FVector StartLoc = TraceCenter;
    FVector EndLoc = TraceCenter + (GetActorForwardVector() * 1.0f);

    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    UKismetSystemLibrary::BoxTraceMulti(
        GetWorld(), StartLoc, EndLoc, BoxHalfSize,
        GetActorRotation(), UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false, ActorsToIgnore, EDrawDebugTrace::None, OutHitResults, true
    );
}

void ABoss::StartFireBreath()
{
    if (bIsDead) return;

    TArray<FHitResult> HitResults;
    PerformBreathTrace(HitResults);

    bool bIsTowerInRange = false;
    for (const FHitResult& Hit : HitResults)
    {
        if (Cast<ATowerBase>(Hit.GetActor()))
        {
            bIsTowerInRange = true;
            break;
        }
    }

    if (!bIsTowerInRange)
    {
        GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, BreathCheckInterval, false);
        return;
    }

    if (GetCharacterMovement()) GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    if (BreathMontage) PlayAnimMontage(BreathMontage);
}

void ABoss::FireBreath()
{
    if (bIsDead) return;

    TArray<FHitResult> HitResults;
    PerformBreathTrace(HitResults);

    for (const FHitResult& Hit : HitResults)
    {
        if (ATowerBase* HitTower = Cast<ATowerBase>(Hit.GetActor()))
        {
            HitTower->ApplyStun(BreathStunDuration);
        }
    }
}

void ABoss::EndFireBreath()
{
    if (bIsDead) return;

    if (GetCharacterMovement())
    {
        if (GameplayTags.HasTagExact(FGridGameplayTags::Get().State_Slow))
            GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * 0.5f;
        else
            GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
    }

    GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, BreathCooldown, false);
}