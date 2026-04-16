

#include "Tower/TowerBase.h"

#include "GridGameplayTags.h"
#include "Components/StaticMeshComponent.h"
#include "Components/DecalComponent.h"
#include "Projectile/ProjectileBase.h"
#include "Enemy/EnemyBase.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h" 
#include "Projectile/PoolManager.h"
#include "Projectile/SplashProjectile.h"
#include "GameplayTagContainer.h"

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
	CachedGridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	
}

void ATowerBase::ApplyStun(float StunDuration)
{
	const FGameplayTag& StunTag = FGridGameplayTags::Get().State_Stun;

	if (!StateTag.HasTagExact(StunTag))
	{
		StateTag.AddTag(StunTag);
        
		// TODO: 타워를 얼음/회색으로 만들거나 스턴 파티클을 켜는 시각적 효과 추가
		UE_LOG(LogTemp, Warning, TEXT("타워 기절! 공격 중지!"));
	}

	GetWorldTimerManager().ClearTimer(StunTimerHandle);
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ATowerBase::EndStun, StunDuration, false);
}

void ATowerBase::EndStun()
{
	const FGameplayTag& StunTag = FGridGameplayTags::Get().State_Stun;
	StateTag.RemoveTag(StunTag);

	UE_LOG(LogTemp, Warning, TEXT("타워 기절 해제! 공격 재개!"));
}

void ATowerBase::FindTarget()
{
    if (!MyData || !CachedGridManager) return;
    
    if (CurrentTarget)
    {
        AEnemyBase* EnemyTarget = Cast<AEnemyBase>(CurrentTarget);
        
    	if (!EnemyTarget || EnemyTarget->IsDead() || FVector::Dist2D(GetActorLocation(), CurrentTarget->GetActorLocation()) > MyData->AttackRange)
        {
            CurrentTarget = nullptr;
        }
        else
        {
        	Fire();
        	return;
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

	if (!bHit || OverlappedActors.Num() == 0) return;

	AEnemyBase* BestTarget = nullptr;
	int32 MinFlowCost = 999999;     // First 모드용
	float MaxHP = -1.0f;           // Strong 모드용
	float MinHP = 999999.f;

	for (AActor* Actor : OverlappedActors)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
		if (!Enemy || Enemy->IsDead()) continue;

		switch (TargetPriority) // 헤더에 만든 Enum
		{
		case ETargetPriority::First:
			{
				int32 EnemyCost = CachedGridManager->GetFlowCost(Enemy->GetActorLocation());
				if (EnemyCost < MinFlowCost)
				{
					MinFlowCost = EnemyCost;
					BestTarget = Enemy;
				}
			}
			break;

		case ETargetPriority::Strong:
			{
				if (Enemy->GetCurrentHP() > MaxHP)
				{
					MaxHP = Enemy->GetCurrentHP();
					BestTarget = Enemy;
				}
			}
			break;

		case ETargetPriority::Weak:
			{
				if (Enemy->GetCurrentHP() < MinHP)
				{
					MinHP = Enemy->GetCurrentHP();
					BestTarget = Enemy;
				}
			}
			break;
		}
	}

	// 4. 타겟 확정 및 발사
	if (BestTarget)
	{
		CurrentTarget = BestTarget;
		Fire();
	}
}

void ATowerBase::Fire()
{
	if (!CurrentTarget || !MyData) return;

	if (StateTag.HasTagExact(FGridGameplayTags::Get().State_Stun)) return;
	
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
