

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
	Super::InitTower(TowerData, bIsPreview);

	IceSphere->SetSphereRadius(MyData->AttackRange);

	if (!bIsPreviewMode)
	{
		GetWorldTimerManager().SetTimer(
			AuraPulseTimer,
			this,
			&AIceTower::PulseAura,
			0.5f,
			true
			);	
	}
}

void AIceTower::PulseAura()
{
	DrawDebugSphere(GetWorld(), GetActorLocation(), IceSphere->GetScaledSphereRadius(), 32, FColor::Blue, false, 0.5f);

	TArray<AActor*> Overlaps;
	IceSphere->GetOverlappingActors(Overlaps, AEnemyBase::StaticClass());

	for (AActor* Actor : Overlaps)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(Actor))
		{
			if (!Enemy->IsDead())
			{
				Enemy->ApplySlow(1.0f);

				// 💡 1. 몬스터의 '현재' 최고 이동 속도를 가져옵니다.
				float CurrentSpeed = Enemy->GetCharacterMovement()->MaxWalkSpeed;

				// 💡 2. FString::Printf를 써서 글자와 숫자를 예쁘게 합칩니다. (%.1f 는 소수점 첫째 자리까지만 보여달라는 뜻!)
				FString DebugMsg = FString::Printf(TEXT("Slowed! Speed: %.1f"), CurrentSpeed);

				// 💡 3. 합성된 문자열을 몬스터 머리 위에 띄웁니다!
				DrawDebugString(
					GetWorld(), 
					Enemy->GetActorLocation() + FVector(0, 0, 100), 
					DebugMsg, 
					nullptr, 
					FColor::Cyan, 
					0.5f
				);
			}
		}
	}
}
