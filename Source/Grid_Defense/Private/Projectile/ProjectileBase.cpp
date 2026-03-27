

#include "Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Enemy/EnemyBase.h"
#include "Projectile/PoolManager.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	ProjectileDamage = 0.0f;

	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	Collision->InitSphereRadius(15.0f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	RootComponent = Collision;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); 

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true; 
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.0f;
	
	ProjectileComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileComponent"));
	ProjectileComponent->SetupAttachment(RootComponent);

	ProjectileComponent->bAutoActivate = true;
	
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		Collision->IgnoreActorWhenMoving(GetOwner(), true);
	}

	if (ProjectileComponent && Projectile)
	{
		ProjectileComponent->SetAsset(Projectile);
	}

	CachedPoolManager = Cast<APoolManager>(UGameplayStatics::GetActorOfClass(GetWorld(), APoolManager::StaticClass()));
	
}

void AProjectileBase::SetDamage(float Damage)
{
	ProjectileDamage = Damage;
}

void AProjectileBase::FireInDirection(const FVector& ShootDirection)
{
	if (Collision)
	{
		Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
		Collision->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics); 
		if (GetOwner())
		{
			Collision->IgnoreActorWhenMoving(GetOwner(), true);
		}
	}

	if (MeshComponent)
	{
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); 
	}

	if (ProjectileComponent)
	{
		ProjectileComponent->Deactivate();   
		ProjectileComponent->Activate(true); 
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->SetUpdatedComponent(Collision); 
       
		ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
		ProjectileMovement->SetComponentTickEnabled(true);
       
		ProjectileMovement->Activate(true); 
	}
}

void AProjectileBase::FireAtTarget(AActor* TargetActor)
{
	if (TargetActor && ProjectileMovement)
	{
		if (AEnemyBase* Enemy = Cast<AEnemyBase>(TargetActor))
		{
			Enemy->OnEnemyDied.AddUniqueDynamic(this, &AProjectileBase::OnTargetDied);
          
			CachedTarget = Enemy;
		}

		FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FireInDirection(Direction);
	}
}

void AProjectileBase::OnTargetDied()
{
	UE_LOG(LogTemp, Error, TEXT("[%s] OnTargetDied 발생! 쫓던 적이 죽어서 돌아감."), *GetName());
	ReturnToManager();
}

void AProjectileBase::ReturnToManager()
{
	UE_LOG(LogTemp, Warning, TEXT("[%s] ReturnToManager 호출됨! (공장으로 돌아감)"), *GetName());
	if (CachedTarget)
	{
		CachedTarget->OnEnemyDied.RemoveDynamic(this, &AProjectileBase::OnTargetDied);
		CachedTarget = nullptr;
	}

	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = FVector::ZeroVector;
	}

	if (ProjectileComponent)
	{
		ProjectileComponent->Deactivate();
	}
	
	if (CachedPoolManager)
	{
		CachedPoolManager->ReturnToPool(this);
	}
	else
	{
		Destroy();
	}
}


void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							FVector NormalImpulse, const FHitResult& Hit)
{
	AEnemyBase* HitEnemy = Cast<AEnemyBase>(OtherActor);

	if (HitEnemy && OtherActor != this && OtherActor != GetOwner())
	{
		UE_LOG(LogTemp, Error, TEXT("[%s] OnHit 발생! 맞은 적: %s"), *GetName(), *HitEnemy->GetName());
		UGameplayStatics::ApplyDamage(
		   OtherActor,                 
		   ProjectileDamage,           
		   nullptr,                    
		   this,                       
		   UDamageType::StaticClass()  
		);

		if (HitEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			   GetWorld(),
			   HitEffect,
			   GetActorLocation(), 
			   GetActorRotation()  
			);
		}
        
		ReturnToManager(); 
	}
}



