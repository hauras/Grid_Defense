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
	StunWidget->SetupAttachment(RootComponent);
	StunWidget->SetWidgetSpace(EWidgetSpace::Screen);
	StunWidget->SetDrawSize(FVector2D(100.f, 40.f));
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

	if (bIsPreviewMode)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		MeshComponent->SetCastShadow(false); 
	}
	else
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		MeshComponent->SetCastShadow(true);
		UpdateAttackTimer();
	}
}

void ATowerBase::ReceiveBuffBroadcast(const FCardData& CardInfo)
{
	bool bIsAllBuff = CardInfo.TowerTag.MatchesTagExact(FGridGameplayTags::Get().Tower_All);
	bool bHasMatchingTag = TowerTag.HasTagExact(CardInfo.TowerTag);

	if (bIsAllBuff || bHasMatchingTag)
	{
		CurrentDamageMultiplier += CardInfo.DamageBuffAmount;

		if (CardInfo.AttackSpeedBuffAmount > 0.0f)
		{
			CurrentAttackSpeedMultiplier += CardInfo.AttackSpeedBuffAmount;
			UpdateAttackTimer();
		}

		if (CardInfo.SplashRadiusBuffAmount > 0.0f)
		{
			CurrentSplashRadiusBonus += CardInfo.SplashRadiusBuffAmount;
		}

		if (CardInfo.ChainCountBuffAmount)
		{
			CurrentChainBonus += CardInfo.ChainCountBuffAmount;
		}
	}
}

void ATowerBase::BeginPlay()
{
	Super::BeginPlay();

	// [수정] null 체크 추가
	CachedPoolManager = Cast<APoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APoolManager::StaticClass()));
	if (!CachedPoolManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] PoolManager를 찾을 수 없습니다."), *GetName());
	}

	CachedGridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (!CachedGridManager)
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] GridManager를 찾을 수 없습니다."), *GetName());
	}

	if (AGridGameState* GS = GetWorld()->GetGameState<AGridGameState>())
	{
		GS->OnBuffUpdated.AddDynamic(this, &ATowerBase::ReceiveBuffBroadcast);

		for (const FCardData& PastCard : GS->AppliedBuff)
		{
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
	int32 MinFlowCost = INVALID_FLOW_COST;
	float MaxHP = -1.0f;
	float MinHP = TNumericLimits<float>::Max();

	for (AActor* Actor : OverlappedActors)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(Actor);
		if (!Enemy || Enemy->IsDead()) continue;

		switch (TargetPriority)
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

	if (!CachedPoolManager) return;

	AActor* PoolActor = CachedPoolManager->GetFromPool(ProjectileClass, SpawnLocation, SpawnRotation);
	AProjectileBase* Projectile = Cast<AProjectileBase>(PoolActor);
	if (!Projectile) return;

	Projectile->SetOwner(this);
	float FinalDamage = MyData->Damage * CurrentDamageMultiplier;

	switch (MyData->TowerType)
	{
	case ETowerType::SingleTarget:
		Projectile->SetDamage(FinalDamage); 
		break;

	case ETowerType::AoE:
		if (ASplashProjectile* SplashProj = Cast<ASplashProjectile>(Projectile))
		{
			float FinalSplashRadius = MyData->SplashRadius + CurrentSplashRadiusBonus;
			SplashProj->InitSplash(FinalSplashRadius, FinalDamage);
		}
		break;

	case ETowerType::Chain:
		break;
	}

	Projectile->FireAtTarget(CurrentTarget);
}

void ATowerBase::UpdateAttackTimer()
{
	if (!MyData || MyData->AttackInterval <= 0.f) return;

	float FinalInterval = MyData->AttackInterval / CurrentAttackSpeedMultiplier;

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ATowerBase::FindTarget, FinalInterval, true);
}
