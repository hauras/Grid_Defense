

#include "Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"

ATowerBase::ATowerBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	//RangeDecal = CreateDefaultSubobject<UDecalComponent>(TEXT("RangeDecal"));
	//RangeDecal->SetupAttachment(RootComponent);
	//RangeDecal->SetRelativeRotation(FRotator(-90.f, 0.f, 0.f));

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

	// 2. 사거리 데칼 설정
	/*if (RangeDecal)
	{
		float Range = MyData->AttackRange;
		// 데칼 크기 적용
		RangeDecal->DecalSize = FVector(200.f, Range * 0.5f, Range * 0.5f);
		RangeDecal->SetVisibility(bIsPreview);
	}*/

	if (bIsPreviewMode)
	{
		// 미리보기일 땐 무조건 겹치기 가능하도록
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false); // 고스트가 그림자 지면 어색할 수 있으니 선택!
	}
}