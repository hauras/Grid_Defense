

#include "Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"

#include "Kismet/KismetSystemLibrary.h" // SphereOverlap 펑션을 쓰기 위해 필요

ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

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
		// [미리보기 모드]
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false); 
	}
	else
	{
		// [실제 설치 모드] (중복 코드 통합!)
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

	UE_LOG(LogTemp, Warning, TEXT("빵야! [%s]에게 데미지를 줍니다!"), *CurrentTarget->GetName());
	DrawDebugLine(GetWorld(), GetActorLocation(), CurrentTarget->GetActorLocation(), FColor::Red, false, 0.5f, 0, 5.0f);
}
