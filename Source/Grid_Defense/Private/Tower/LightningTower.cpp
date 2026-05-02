#include "Tower/LightningTower.h"
#include "NiagaraFunctionLibrary.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Projectile/EffectActor.h"
#include "Projectile/PoolManager.h"
#include "GridGameplayTags.h"

ALightningTower::ALightningTower()
{
}

void ALightningTower::Fire()
{
	if (!CurrentTarget) return;
	if (StateTag.HasTagExact(FGridGameplayTags::Get().State_Stun)) return;

	TArray<AActor*> HitActors;
	ExecuteChain(CurrentTarget, 0, HitActors);
}

void ALightningTower::ExecuteChain(AActor* Target, int32 CurrentChainCount, TArray<AActor*> HitActors)
{
	if (!IsValid(Target) || !MyData) return;

	const int32 FinalMaxChain = MyData->ChainCount + CurrentChainBonus;
	const float FinalChainRange = MyData->ChainRange;

	if (CurrentChainCount >= FinalMaxChain || HitActors.Contains(Target)) return;

	const float FinalDamage = MyData->Damage * CurrentDamageMultiplier;
	UGameplayStatics::ApplyDamage(
		Target,
		FinalDamage,
		GetInstigatorController(),
		this,
		UDamageType::StaticClass()
	);

	if (LightningEffectClass && CachedPoolManager)
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
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(Target);

	UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		Target->GetActorLocation(),
		FinalChainRange,
		ObjectTypes,
		AEnemyBase::StaticClass(),
		ActorsToIgnore,
		Overlaps
	);

	AActor* NextTarget = nullptr;
	float MinDistance = FinalChainRange + 1.0f;

	for (AActor* OverlapActor : Overlaps)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(OverlapActor);
		if (!Enemy || Enemy->IsDead() || HitActors.Contains(Enemy)) continue;

		float Distance = FVector::Distance(Target->GetActorLocation(), Enemy->GetActorLocation());
		if (Distance < MinDistance)
		{
			MinDistance = Distance;
			NextTarget = Enemy;
		}
	}

	if (NextTarget)
	{
		FTimerHandle& NewHandle = ChainTimerHandles.AddDefaulted_GetRef();
		FTimerDelegate ChainDelegate;
		ChainDelegate.BindUObject(this, &ALightningTower::ExecuteChain, NextTarget, CurrentChainCount + 1, HitActors);
		GetWorld()->GetTimerManager().SetTimer(NewHandle, ChainDelegate, 0.15f, false);
	}
}