

#include "Tower/TowerBase.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Projectile/ProjectileBase.h"
#include "Enemy/EnemyBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Projectile/PoolManager.h"
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

	// 가중치 사용해서 사거리 표시
	if (RangeDecal)
	{
		RangeDecal->SetUsingAbsoluteScale(true); 
       
		float VisualRadius = MyData->AttackRange * MyData->DecalMultiplier;

		RangeDecal->DecalSize = FVector(2000.0f, VisualRadius, VisualRadius);

		if (MyData->RangeDecalMaterial)
		{
			RangeDecal->SetDecalMaterial(MyData->RangeDecalMaterial);
		}
		RangeDecal->SetVisibility(bIsPreviewMode);
	}

	// 2. 상태 설정 및 타이머
	if (bIsPreviewMode)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false); 
	}
	else
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCastShadow(true);
        
		if (MyData && MyData->AttackInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ATowerBase::FindTarget, MyData->AttackInterval, true);
		}
	}
}

void ATowerBase::BeginPlay()
{
	Super::BeginPlay();

	CachedPoolManager = Cast<APoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APoolManager::StaticClass()));
	if (!CachedPoolManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] 풀 매니저를 찾을 수 없습니다! 맵에 배치했는지 확인하세요."), *GetName());
	}
}

void ATowerBase::FindTarget()
{
    if (!MyData) return;
    
    if (CurrentTarget)
    {
        AEnemyBase* EnemyTarget = Cast<AEnemyBase>(CurrentTarget);
        
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

    //  새로운 타겟 탐색 
    TArray<AActor*> OverlappedActors;
    TArray<AActor*> ActorsToIgnore;
    ActorsToIgnore.Add(this);

    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));

    bool bHit = UKismetSystemLibrary::SphereOverlapActors(
       this,
       GetActorLocation(), 
       MyData->AttackRange + 300.0f, 
       ObjectTypes,
       AEnemyBase::StaticClass(), 
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
             float Dist2D = FVector::Dist2D(GetActorLocation(), Enemy->GetActorLocation());
             
             if (Dist2D <= MyData->AttackRange)
             {
                CurrentTarget = Enemy; 
                Fire();               
                break; 
             }
          }
       }
    }
}

void ATowerBase::Fire()
{
	if (!CurrentTarget || !MyData) return;

	if (AttackSound) 
	{
		UGameplayStatics::PlaySoundAtLocation(this, AttackSound, GetActorLocation());
	}
	
	FVector SpawnLocation = MeshComponent->GetSocketLocation(TEXT("Fire_Socket"));
	FRotator SpawnRotation = MeshComponent->GetSocketRotation(TEXT("Fire_Socket"));

	AActor* PoolActor = CachedPoolManager->GetFromPool(ProjectileClass, SpawnLocation, SpawnRotation);
	AProjectileBase* Projectile = Cast<AProjectileBase>(PoolActor);
	if (Projectile)
	{
		Projectile->SetOwner(this);
		
		switch (MyData->TowerType)
		{
		case ETowerType::SingleTarget:
			Projectile->SetDamage(MyData->Damage);
			break;

		case ETowerType::AoE:
			if (ASplashProjectile* SplashProj = Cast<ASplashProjectile>(Projectile))
			{
				SplashProj->InitSplash(MyData->SplashRadius, MyData->Damage);
			}
			break;

		case ETowerType::Chain:
			Projectile->SetDamage(MyData->Damage);
			break;
		}

		Projectile->FireAtTarget(CurrentTarget);
	}
}
