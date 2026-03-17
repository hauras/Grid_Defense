

#include "Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"

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

	// 1. 외형 설정
	if (MyData->PreviewMesh)
	{
		MeshComponent->SetStaticMesh(MyData->PreviewMesh);
	}

	// 2. 💡 사거리 표시기(데칼) 설정 (추가된 로직)
	if (RangeDecal)
	{
		// 데이터 에셋의 AttackRange 값에 맞춰 데칼 크기 조절
		// DecalSize의 Y, Z가 원의 반지름(Radius)이 됩니다.
		float Radius = MyData->AttackRange; 
		RangeDecal->DecalSize = FVector(100.0f, Radius, Radius);

		// 데이터 에셋에 설정한 머티리얼 입히기
		if (MyData->RangeDecalMaterial)
		{
			RangeDecal->SetDecalMaterial(MyData->RangeDecalMaterial);
		}

		// 프리뷰 모드일 때만 사거리를 보여줌
		RangeDecal->SetVisibility(bIsPreviewMode);
	}

	// 3. 상태에 따른 설정
	if (bIsPreviewMode)
	{
		// 미리보기 모드
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false); 
	}
	else
	{
		// 실제 설치 모드
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCastShadow(true);
		// 설치 완료 후에는 사거리 표시를 끄고 싶다면 아래 추가
		// RangeIndicator->SetVisibility(false);
	}
}