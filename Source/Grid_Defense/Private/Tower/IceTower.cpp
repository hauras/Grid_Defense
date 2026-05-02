#include "Tower/IceTower.h"
#include "NiagaraComponent.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/KismetSystemLibrary.h"
#include "DrawDebugHelpers.h"

AIceTower::AIceTower()
{
    MistEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistEffect"));
    MistEffect->SetupAttachment(Root);
    MistEffect->bAutoActivate = false;
}

void AIceTower::InitTower(UTowerData* TowerData, bool bIsPreview)
{
    Super::InitTower(TowerData, bIsPreview);

    if (!MyData) return;

    if (!bIsPreviewMode)
    {
        GetWorldTimerManager().ClearTimer(AttackTimerHandle);

        // AttackRange 2000 기준으로 스케일 계산
        float EffectScale = MyData->AttackRange / 2000.0f;
        MistEffect->SetWorldScale3D(FVector(EffectScale));
        MistEffect->Activate();

        GetWorldTimerManager().SetTimer(AuraPulseTimer, this, &AIceTower::PulseAura, 0.5f, true);
    }
}

void AIceTower::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    GetWorldTimerManager().ClearTimer(AuraPulseTimer);
}

void AIceTower::PulseAura()
{
    if (!MyData) return;

    // 디버그 - 실제 슬로우 범위 (파란색)
    DrawDebugCircle(
        GetWorld(),
        GetActorLocation(),
        MyData->AttackRange,
        64,
        FColor::Blue,
        false,
        0.5f,
        0,
        5.0f,
        FVector(1, 0, 0),
        FVector(0, 1, 0)
    );

    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    constexpr float OverlapMargin = 200.0f;
    UKismetSystemLibrary::SphereOverlapActors(
        this,
        GetActorLocation(),
        MyData->AttackRange + OverlapMargin,
        ObjectTypes,
        AEnemyBase::StaticClass(),
        ActorsToIgnore,
        OverlappedActors
    );

    for (AActor* Actor : OverlappedActors)
    {
        if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
        {
            if (!Enemy->IsDead())
            {
                float Dist2D = FVector::Dist2D(GetActorLocation(), Enemy->GetActorLocation());
                if (Dist2D <= MyData->AttackRange)
                {
                    Enemy->ApplySlow(SlowDuration);
                }
            }
        }
    }
}