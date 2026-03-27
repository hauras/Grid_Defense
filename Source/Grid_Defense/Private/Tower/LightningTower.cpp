

#include "Tower/LightningTower.h"
#include "NiagaraFunctionLibrary.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/EffectActor.h"
#include "Projectile/PoolManager.h"

ALightningTower::ALightningTower()
{
}

void ALightningTower::Fire()
{
	if (!CurrentTarget) return;
	TArray<AActor*> HitActors;

	ExecuteChain(CurrentTarget, 0, HitActors);
}

void ALightningTower::ExecuteChain(AActor* Target, int32 CurrentChainCount, TArray<AActor*> HitActors) // & 삭제!
{
	if (!IsValid(Target) || CurrentChainCount >= MaxChainCount || HitActors.Contains(Target) ) return;

	if (MyData)
	{
		UGameplayStatics::ApplyDamage(
			Target,
			MyData->Damage,
			GetInstigatorController(),
			this,
			UDamageType::StaticClass()
			);
	}

	if (LightningEffectClass) 
	{
		AActor* PooledActor = CachedPoolManager->GetFromPool(LightningEffectClass, Target->GetActorLocation(), FRotator::ZeroRotator);
   
		if (AEffectActor* Effect = Cast<AEffectActor>(PooledActor))
		{
			Effect->PlayEffect(0.2f); 
		}
	}

	HitActors.Add(Target);

	TArray<AActor*> Overlaps;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn)); // 몬스터가 보통 Pawn 채널이므로 세팅

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Target);

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Target->GetActorLocation(),
		ChainRange,
		ObjectTypes,
		AEnemyBase::StaticClass(),
		ActorsToIgnore,
		Overlaps
		);

	AActor* NextTarget = nullptr;
	float MinDistance = ChainRange + 1.0f;

	for (AActor* OverlapActor : Overlaps)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(OverlapActor);

		if (Enemy && !Enemy->IsDead() && !HitActors.Contains(Enemy))
		{
			float Distance = FVector::Distance(Target->GetActorLocation(), Enemy->GetActorLocation());

			if (Distance < MinDistance)
			{
				MinDistance = Distance;
				NextTarget = Enemy;
			}
		}
	}

	if (NextTarget)
	{

		FTimerHandle ChainTimerHandle;
		FTimerDelegate ChainDelegate;

		ChainDelegate.BindUObject(this, &ALightningTower::ExecuteChain, NextTarget, CurrentChainCount + 1, HitActors);

		GetWorld()->GetTimerManager().SetTimer(ChainTimerHandle, ChainDelegate, 0.15f, false);
	}
}
