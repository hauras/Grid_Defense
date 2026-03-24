

#include "Tower/IceTower.h"
#include "NiagaraComponent.h"
#include "Components/SphereComponent.h"
#include "Enemy/EnemyBase.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"

AIceTower::AIceTower()
{
	IceSphere = CreateDefaultSubobject<USphereComponent>("IceSphere");
	IceSphere->SetupAttachment(Root);

	MistEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MistEffect"));
	MistEffect->SetupAttachment(Root);
	MistEffect->bAutoActivate = false;
	
}

void AIceTower::BeginPlay()
{
	Super::BeginPlay();
}

void AIceTower::Fire()
{
	
}

void AIceTower::InitTower(UTowerData* TowerData, bool bIsPreview)
{
	// 1. 부모의 InitTower 먼저 호출 (데칼 설정 로직 실행)
	Super::InitTower(TowerData, bIsPreview);

	if (!MyData) return;

	// 💡 [핵심 수정] 부모의 데칼 계산 방식과 일치시킵니다.
	// 부모가 AdjustedRadius를 구할 때 (Range / Scale)을 했던 것처럼
	// 콜리전 컴포넌트도 스케일 보정을 해줘야 월드 사거리와 일치합니다.
	float CurrentScale = GetActorScale3D().X;
	if (CurrentScale > 0.f)
	{
		// 월드 사거리 값을 로컬 스케일로 나눠서 셋팅
		IceSphere->SetSphereRadius(MyData->AttackRange / CurrentScale);
	}

	if (!bIsPreviewMode) 
	{
		MistEffect->Activate(); 
		GetWorldTimerManager().SetTimer(AuraPulseTimer, this, &AIceTower::PulseAura, 0.5f, true); 
	}
	else 
	{
		IceSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}

void AIceTower::PulseAura()
{
	// 💡 디버그 구체도 이제 정확한 월드 사거리를 그려야 합니다.
	// IceSphere->GetScaledSphereRadius()를 쓰면 현재 월드상의 실제 판정 크기가 나옵니다.
	float RealWorldRadius = IceSphere->GetScaledSphereRadius();

	DrawDebugSphere(
		GetWorld(), 
		GetActorLocation(), 
		RealWorldRadius, 
		32,                                 
		FColor::Blue,                      
		false,                            
		0.5f                                
	);

	TArray<AActor*> Overlaps;
	IceSphere->GetOverlappingActors(Overlaps, AEnemyBase::StaticClass());

	for (AActor* Actor : Overlaps)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
		{
			if (!Enemy->IsDead())
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
