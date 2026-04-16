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

void ABoss::StartFireBreath()
{
    if (bIsDead) return;

    FVector BoxHalfSize = FVector(BreathRange * 0.5f, BreathWidth, 100.f);
    FVector TraceCenter = GetActorLocation() + (GetActorForwardVector() * BoxHalfSize.X);
    FVector StartLoc = TraceCenter;
    FVector EndLoc = TraceCenter + (GetActorForwardVector() * 1.0f);

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    bool bHit = UKismetSystemLibrary::BoxTraceMulti(
       GetWorld(), StartLoc, EndLoc, BoxHalfSize,
       GetActorRotation(), UEngineTypes::ConvertToTraceType(ECC_Visibility), 
       false, ActorsToIgnore, EDrawDebugTrace::None, HitResults, true
    );

    bool bIsTowerInRange = false;

    if (bHit)
    {
       for (const FHitResult& Hit : HitResults)
       {
          if (Cast<ATowerBase>(Hit.GetActor()))
          {
             bIsTowerInRange = true;
             break; 
          }
       }
    }

    // 💡 [수정 완료!!] 타워가 안 보인다면 타이머를 살려둡니다! (0.5초 뒤에 다시 검사)
    if (!bIsTowerInRange)
    {
       GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, 0.5f, false);
       return; 
    }

    // 타워를 발견했다면 멈춰 서서 브레스 시작!
    if (GetCharacterMovement()) GetCharacterMovement()->MaxWalkSpeed = 0.0f;
    if (BreathMontage) PlayAnimMontage(BreathMontage);
}

void ABoss::FireBreath()
{
    if (bIsDead) 
    {
       return;
    }
    
    FVector BoxHalfSize = FVector(BreathRange * 0.5f, BreathWidth, 100.f);
    FVector TraceCenter = GetActorLocation() + (GetActorForwardVector() * BoxHalfSize.X);

    FVector StartLoc = TraceCenter;
    FVector EndLoc = TraceCenter + (GetActorForwardVector() * 1.0f);

    TArray<FHitResult> HitResults;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    bool bHit = UKismetSystemLibrary::BoxTraceMulti(
       GetWorld(), StartLoc, EndLoc, BoxHalfSize,
       GetActorRotation(), UEngineTypes::ConvertToTraceType(ECC_Visibility), 
       false, ActorsToIgnore, EDrawDebugTrace::ForDuration, HitResults, true,
       FLinearColor::Red, FLinearColor::Green, 2.0f
    );

    if (bHit)
    {
       for (const FHitResult& Hit : HitResults)
       {
          if (ATowerBase* HitTower = Cast<ATowerBase>(Hit.GetActor()))
          {
             HitTower->ApplyStun(3.0f); 
          }
       }
    }
}

void ABoss::EndFireBreath()
{
    if (bIsDead) return;

    // 공격 종료 후 이동 속도 복구
    if (GetCharacterMovement())
    {
       if (GameplayTags.HasTagExact(FGridGameplayTags::Get().State_Slow))
          GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * 0.5f;
       else
          GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
    }

    // 💡 공격이 끝난 이 시점부터 다시 3초(쿨다운)를 셉니다.
    GetWorldTimerManager().SetTimer(BreathCooldownTimer, this, &ABoss::StartFireBreath, BreathCooldown, false);
}