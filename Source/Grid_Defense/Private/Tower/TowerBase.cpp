

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
#include "Components/WidgetComponent.h"
#include "State/GridGameState.h"

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

	StunWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("StunWidget"));
	StunWidget->SetupAttachment(RootComponent); // 포탑 몸체에 부착
	StunWidget->SetWidgetSpace(EWidgetSpace::Screen); // 항상 카메라를 바라보게 설정
	StunWidget->SetDrawSize(FVector2D(100.f, 40.f)); // 글씨 크기에 맞게 조절
	StunWidget->SetVisibility(false);
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

void ATowerBase::ReceiveBuffBroadcast(const FCardData& CardInfo)
{
	// 1. 전체 버프(Tower_All)인지 먼저 확인
	bool bIsAllBuff = CardInfo.TowerTag.MatchesTagExact(FGridGameplayTags::Get().Tower_All);
    
	// 2. 내 가방(TowerTags) 안에 카드가 찾는 태그(CardInfo.TowerTag)가 들어있는지 확인
	bool bHasMatchingTag = TowerTag.HasTagExact(CardInfo.TowerTag);

	if (bIsAllBuff || bHasMatchingTag)
	{
		// 내 스탯에 버프 추가
		CurrentDamageMultiplier += CardInfo.DamageBuffAmount;
        
		UE_LOG(LogTemp, Warning, TEXT("[%s] 카드 버프 적용 성공: %s"), *GetName(), *CardInfo.CardName);
	}
}

void ATowerBase::BeginPlay()
{
	Super::BeginPlay();

	CachedPoolManager = Cast<APoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APoolManager::StaticClass()));
	CachedGridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	if (AGridGameState* GS = GetWorld()->GetGameState<AGridGameState>())
	{
		GS->OnBuffUpdated.AddDynamic(this, &ATowerBase::ReceiveBuffBroadcast);

		for (const FCardData& PastCard : GS->AppliedBuff)
		{
			// 자기 자신의 수신기를 강제로 호출해서 과거 데이터를 다 먹습니다.
			ReceiveBuffBroadcast(PastCard);
		}
	}
}

void ATowerBase::ApplyStun(float StunDuration)
{
	const FGameplayTag& StunTag = FGridGameplayTags::Get().State_Stun;

	if (!StateTag.HasTagExact(StunTag))
	{
		StateTag.AddTag(StunTag);
        
		if (StunWidget)
		{
			StunWidget->SetVisibility(true);
		}
	}

	GetWorldTimerManager().ClearTimer(StunTimerHandle);
	GetWorldTimerManager().SetTimer(StunTimerHandle, this, &ATowerBase::EndStun, StunDuration, false);
}

void ATowerBase::EndStun()
{
	const FGameplayTag& StunTag = FGridGameplayTags::Get().State_Stun;
	StateTag.RemoveTag(StunTag);

	if (StunWidget)
	{
		StunWidget->SetVisibility(false);
	}
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
       
		// ==========================================
		// 💡 [여기에 추가!] 오리지널 데미지에 현재 버프 배율을 곱해서 최종 데미지 계산!
		float FinalDamage = MyData->Damage * CurrentDamageMultiplier;
		// ==========================================

		switch (MyData->TowerType)
		{
		case ETowerType::SingleTarget:
			// 💡 MyData->Damage 대신 방금 계산한 FinalDamage를 줍니다.
			Projectile->SetDamage(FinalDamage); 
			break;

		case ETowerType::AoE:
			if (ASplashProjectile* SplashProj = Cast<ASplashProjectile>(Projectile))
			{
				// 💡 여기도 FinalDamage!
				SplashProj->InitSplash(MyData->SplashRadius, FinalDamage); 
			}
			break;

		case ETowerType::Chain:
			// 💡 여기도 FinalDamage!
			Projectile->SetDamage(FinalDamage); 
			break;
		}

		Projectile->FireAtTarget(CurrentTarget);
	}
}