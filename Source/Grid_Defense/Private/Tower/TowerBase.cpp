

#include "Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Projectile/ProjectileBase.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Projectile/SplashProjectile.h"

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
	RangeDecal->SetVisibility(false); 

}

void ATowerBase::InitTower(UTowerData* TowerData, bool bIsPreview)
{
	if (!TowerData) return;
	MyData = TowerData;
	bIsPreviewMode = bIsPreview;

	if (bIsPreviewMode && MyData->PreviewMesh)
	{
		MeshComponent->SetStaticMesh(MyData->PreviewMesh);
	}

	// 2. 사거리 표시 
	if (RangeDecal)
	{
		float CurrentScale = GetActorScale3D().X; 
		
		// 보정치 추가
		float DecalMultiplier = 3.5f; 
    
		float AdjustedRadius = (MyData->AttackRange / CurrentScale) * DecalMultiplier;

		RangeDecal->DecalSize = FVector(2000.0f, AdjustedRadius, AdjustedRadius);

		if (MyData->RangeDecalMaterial)
		{
			RangeDecal->SetDecalMaterial(MyData->RangeDecalMaterial);
		}
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
        AEnemyBase* EnemyTarget = Cast<AEnemyBase>(CurrentTarget);
        
        // 타겟이 이미 죽었으면 타겟 초기화
        if (!EnemyTarget || EnemyTarget->IsDead())
        {
            CurrentTarget = nullptr;
        }
        else
        {
            
            float Distance2D = FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation());
            
            if (Distance2D <= MyData->AttackRange)
            {
                Fire();
                return;
            }
            else
            {
                CurrentTarget = nullptr;
            }
        }
    }

   // 새로운 타겟 찾기
    FVector SearchLocation = GetActorLocation();
    SearchLocation.Z = 0.0f; 

    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
       this,
       SearchLocation,
       MyData->AttackRange,
       ObjectTypes,
       AActor::StaticClass(),
       ActorsToIgnore,
       OverlappedActors
    );

    if (bHit && OverlappedActors.Num() > 0)
    {
       for (AActor* Actor : OverlappedActors)
       {
          AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
            
          if (Enemy && !Enemy->IsDead()) 
          {
          	
             CurrentTarget = Enemy; 
             Fire();               
             break;                
          }
       }
    }
}

void ATowerBase::Fire()
{
	if (!CurrentTarget || !MyData) return;

	// 1. 소환 위치/회전 계산
	FVector SpawnLocation = MeshComponent->GetSocketLocation(TEXT("Fire_Socket"));
	FRotator SpawnRotation = MeshComponent->GetSocketRotation(TEXT("Fire_Socket"));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 2. 투사체 소환
	AProjectileBase* Projectile = GetWorld()->SpawnActor<AProjectileBase>(ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Projectile)
	{
		// 3. 💡 TowerData의 'TowerType'에 따라 투사체 세팅을 다르게 함
		switch (MyData->TowerType)
		{
		case ETowerType::SingleTarget:
			// 일반 타워: 
			Projectile->SetDamage(MyData->Damage);
			break;

		case ETowerType::AoE:
			if (ASplashProjectile* SplashProj = Cast<ASplashProjectile>(Projectile))
			{
				SplashProj->InitSplash(MyData->SplashRadius, MyData->Damage);
			}
			break;

		case ETowerType::Chain:
			// TODO: 나중에 체인 타워 로직 추가 (예: 전이 횟수 등)
			Projectile->SetDamage(MyData->Damage);
			break;
		}

		// 4. 발사!
		Projectile->FireAtTarget(CurrentTarget);
	}
}
