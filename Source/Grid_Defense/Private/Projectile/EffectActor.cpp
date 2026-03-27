#include "Projectile/EffectActor.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/PoolManager.h" // 풀 매니저 경로 확인!

AEffectActor::AEffectActor()
{
	PrimaryActorTick.bCanEverTick = false;

	NiagaraComp = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComp"));
	RootComponent = NiagaraComp;
    
	NiagaraComp->bAutoActivate = false;
}

void AEffectActor::BeginPlay()
{
	Super::BeginPlay();

	CachedPoolManager = Cast<APoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APoolManager::StaticClass()));
}

void AEffectActor::PlayEffect(float LifeTime)
{
	SetActorHiddenInGame(false);
    
	if (NiagaraComp)
	{
		NiagaraComp->Activate(true); // 처음부터 다시 재생
	}

	GetWorldTimerManager().SetTimer(ReturnTimerHandle, this, &AEffectActor::ReturnToManager, LifeTime, false);
}

void AEffectActor::ReturnToManager()
{
	GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

	// 6. 이펙트 끄기
	if (NiagaraComp)
	{
		NiagaraComp->Deactivate();
	}

	// 7. 바구니(풀)로 반납
	if (CachedPoolManager)
	{
		CachedPoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}
