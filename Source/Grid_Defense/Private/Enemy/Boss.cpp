#include "Enemy/Boss.h"

#include "GridGameplayTags.h"
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Tower/TowerBase.h"

ABoss::ABoss()
{
}

void ABoss::BeginPlay()
{
    Super::BeginPlay();
    GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, BreathCooldown, false);
}

void ABoss::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(BreathCooldownTimer);
}

// [수정] 중복 BoxTrace 로직 분리
TArray<FHitResult> ABoss::PerformBreathTrace()
{
    FVector BoxHalfSize = FVector(BreathRange * 0.5f, BreathWidth, 100.f);
    FVector TraceCenter = GetActorLocation() + (GetActorForwardVector() * BoxHalfSize.X);
    FVector StartLoc = TraceCenter;
    FVector EndLoc = TraceCenter + (GetActorForwardVector() * 1.0f);

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    UKismetSystemLibrary::BoxTraceMulti(
        GetWorld(), StartLoc, EndLoc, BoxHalfSize,
        GetActorRotation(), UEngineTypes::ConvertToTraceType(ECC_Visibility),
        false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true
    );

    return HitResults;
}

void ABoss::StartFireBreath()
{
    if (bIsDead) return;

    TArray<FHitResult> HitResults = PerformBreathTrace();

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
        // [수정] 0.5f → BreathCheckInterval
        GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, BreathCheckInterval, false);
        return;
    }

    if (GetCharacterMovement()) GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    if (BreathMontage) PlayAnimMontage(BreathMontage);
}

void ABoss::FireBreath()
{
    if (bIsDead) return;

    TArray<FHitResult> HitResults = PerformBreathTrace();

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