#include "Tower/IceTower.h"
#include "NiagaraComponent.h"
#include "Enemy/EnemyBase.h"
#include "DrawDebugHelpers.h"
#include "Kismet/KismetSystemLibrary.h" // 오버랩 함수용

AIceTower::AIceTower()
{
    // 💡 IceSphere 컴포넌트 생성 코드 삭제! 엄청 가벼워졌습니다.

    MistEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistEffect"));
    MistEffect->SetupAttachment(Root);
    MistEffect->bAutoActivate = false;
}

void AIceTower::InitTower(UTowerData* TowerData, bool bIsPreview)
{
    Super::InitTower(TowerData, bIsPreview);

    if (!MyData) return;

    // 💡 IceSphere 크기 조절 코드도 삭제! (데칼 세팅은 Super::InitTower가 다 해줌)

    if (!bIsPreviewMode) 
    {
       MistEffect->Activate(); 
       GetWorldTimerManager().SetTimer(AuraPulseTimer, this, &AIceTower::PulseAura, 0.5f, true); 
    }
}

void AIceTower::PulseAura()
{
    if (!MyData) return;

    // 1. 디버그 구체 (절대적인 데이터 사거리 기준)
    DrawDebugSphere(
       GetWorld(), 
       GetActorLocation(), 
       MyData->AttackRange, // 스케일 꼬임 없이 순수 데이터 사거리 사용
       32,                                 
       FColor::Blue,                      
       false,                            
       0.5f                                
    );

    // 2. 주변 탐색 (부모 타워와 똑같이 넉넉하게 긁어옵니다)
    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    UKismetSystemLibrary::SphereOverlapActors(
       this,
       GetActorLocation(),
       MyData->AttackRange + 200.0f, // 실제보다 넓게 탐색 (3D 오차 방지)
       ObjectTypes,
       AEnemyBase::StaticClass(),
       ActorsToIgnore,
       OverlappedActors
    );

    // 3. 2D 거리로 정확하게 솎아내기 (선을 밟은 적만 슬로우!)
    for (AActor* Actor : OverlappedActors)
    {
       if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
       {
          if (!Enemy->IsDead())
          {
             // 💡 2D 평면 거리 검증! 데칼 원 안에 완벽히 들어왔는가?
             float Dist2D = FVector::Dist2D(GetActorLocation(), Enemy->GetActorLocation());
             
             if (Dist2D <= MyData->AttackRange)
             {
                 // 여기서 슬로우 로직 실행
                 Enemy->ApplySlow(1.0f);

                 DrawDebugString(
                    GetWorld(), 
                    Enemy->GetActorLocation() + FVector(0, 0, 100), 
                    TEXT("Slowed!"), 
                    nullptr, 
                    FColor::Cyan, 
                    0.5f
                 );
             }
          }
       }
    }
}