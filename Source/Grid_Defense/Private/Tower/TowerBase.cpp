

#include "Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Projectile/ProjectileBase.h"

#include "Kismet/KismetSystemLibrary.h" 

ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = false;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(Root);
	
	RangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecal"));
	RangeDecal->SetupAttachment(RootComponent);
	RangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));
	RangeDecal->SetVisibility(false); // 기본은 꺼둠

}

void ATowerBase::InitTower(UTowerData* TowerData, bool bIsPreview)
{
	if (!TowerData) return;
	MyData = TowerData;
	bIsPreviewMode = bIsPreview;

	// 1. 외형 설정 (고스트 모드일 때만 PreviewMesh 사용)
	// 💡 만약 진짜 타워도 PreviewMesh랑 똑같이 생겼다면 조건문 없이 쓰셔도 됩니다.
	if (bIsPreviewMode && MyData->PreviewMesh)
	{
		MeshComponent->SetStaticMesh(MyData->PreviewMesh);
	}

	// 2. 사거리 표시 (데칼)
	if (RangeDecal)
	{
		float Radius = MyData->AttackRange; 
		RangeDecal->DecalSize = FVector(100.0f, Radius, Radius);

		if (MyData->RangeDecalMaterial)
		{
			RangeDecal->SetDecalMaterial(MyData->RangeDecalMaterial);
		}

		// 프리뷰 모드일 때만 사거리를 보여줌
		RangeDecal->SetVisibility(bIsPreviewMode);
	}

	// 3. 상태에 따른 설정 및 타이머 작동
	if (bIsPreviewMode)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false); 
	}
	else
	{
		// [실제 설치 모드] 
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCastShadow(true);
        

		if (MyData && MyData->AttackInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ATowerBase::FindTarget, MyData->AttackInterval, true);
		}
	}
}

void ATowerBase::FindTarget()
{
	if (!MyData) return;

	if (CurrentTarget)
	{
		float Distance = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());
		if (Distance <= MyData->AttackRange)
		{
			Fire();
			return;
		}
		else
		{
			CurrentTarget = nullptr;
		}
	}

	TArray<AActor*> OverlappedActors;
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(this);

	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

	bool bHit = UKismetSystemLibrary::SphereOverlapActors(
		this,
		GetActorLocation(),
		MyData->AttackRange,
		ObjectTypes,
		AActor::StaticClass(),
		ActorsToIgnore,
		OverlappedActors
		);

	if (bHit && OverlappedActors.Num() > 0)
	{
		CurrentTarget = OverlappedActors[0];
		Fire();
	}
}

void ATowerBase::Fire()
{
	if (!CurrentTarget) return;

	DrawDebugLine(GetWorld(), GetActorLocation(), CurrentTarget->GetActorLocation(), FColor::Red, false, 0.5f, 0, 5.0f);

	if (ProjectileClass)
	{
		// 2. 발사구(소켓) 위치와 회전값 가져오기! 
		// (주의: MeshComponent 이름이 정빈님 코드의 타워 메쉬 변수명과 일치해야 합니다)
		FVector SpawnLocation = MeshComponent->GetSocketLocation(TEXT("Fire_Socket"));
		FRotator SpawnRotation = MeshComponent->GetSocketRotation(TEXT("Fire_Socket"));

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;

		// 3. 드디어 마법 구슬 스폰!!
		AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

		if (Projectile)
		{
			// 4. 총알에게 타워의 공격력(데미지)을 쥐여줍니다. 
			// (임시로 10.0f를 넣었지만, 나중에 타워 스탯 변수로 바꾸세요!)
			Projectile->SetDamage(MyData->Damage);

			// 5. 타겟을 향해 날아가라!!
			Projectile->FireAtTarget(CurrentTarget);
		}
	}
}
