

#include "Tower/LightningTower.h"
#include "NiagaraFunctionLibrary.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/GameplayStatics.h"

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

	if (LightningEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), LightningEffect, Target->GetActorLocation());
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

		// 💡 [새로운 로직] 0.15초 딜레이를 주는 타이머 세팅
		FTimerHandle ChainTimerHandle;
		FTimerDelegate ChainDelegate;

		// 델리게이트에 '실행할 함수'와 '넘겨줄 파라미터들'을 포장합니다.
		ChainDelegate.BindUObject(this, &ALightningTower::ExecuteChain, NextTarget, CurrentChainCount + 1, HitActors);

		// 타이머 매니저에게 0.15초(0.15f) 뒤에, 단 한 번(false) 실행하라고 명령합니다.
		GetWorld()->GetTimerManager().SetTimer(ChainTimerHandle, ChainDelegate, 0.15f, false);
	}
}
