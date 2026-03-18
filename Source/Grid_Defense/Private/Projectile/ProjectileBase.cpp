

#include "Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraSystem.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

AProjectileBase::AProjectileBase()
{
	PrimaryActorTick.bCanEverTick = false;
	ProjectileDamage = 0.0f; // 기본 데미지

	// 1. 충돌체 세팅
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	Collision->InitSphereRadius(15.0f);
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->OnComponentHit.AddDynamic(this, &AProjectileBase::OnHit);
	RootComponent = Collision;

	// 2. 외형 세팅
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 충돌은 CollisionComp가 담당

	// 3. 움직임 세팅 (순수 직선 비행)
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = Collision;
	ProjectileMovement->InitialSpeed = 1000.f;
	ProjectileMovement->MaxSpeed = 1000.f;
	ProjectileMovement->bRotationFollowsVelocity = true; // 날아가는 방향 바라보기
	ProjectileMovement->bShouldBounce = false;

	ProjectileComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("ProjectileComponent"));
	ProjectileComponent->SetupAttachment(RootComponent);

	ProjectileComponent->bAutoActivate = true;
	
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		// 내 충돌체(Collision)가 날아갈 때 주인은 그냥 무시하고 통과해라!
		Collision->IgnoreActorWhenMoving(GetOwner(), true);
	}

	if (ProjectileComponent && Projectile)
	{
		ProjectileComponent->SetAsset(Projectile);
	}
}

void AProjectileBase::SetDamage(float Damage)
{
	ProjectileDamage = Damage;
}

void AProjectileBase::FireInDirection(const FVector& ShootDirection)
{
	if (ProjectileMovement)
	{
		ProjectileMovement->Velocity = ShootDirection * ProjectileMovement->InitialSpeed;
	}
}

void AProjectileBase::FireAtTarget(AActor* TargetActor)
{
	if (TargetActor && ProjectileMovement)
	{
		FVector Direction = (TargetActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
		FireInDirection(Direction);
	}
}

void AProjectileBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != this && OtherActor != GetOwner())
	{
		UGameplayStatics::ApplyDamage(
		   OtherActor,                 
		   ProjectileDamage,           
		   nullptr,                    
		   this,                       
		   UDamageType::StaticClass()  
		);

		// 💡 [추가된 부분] 파괴되기 직전에 현재 위치에 피격 이펙트 소환!
		// (주의: ProjectileBase.h 에 class UNiagaraSystem* HitEffect; 가 선언되어 있어야 합니다)
		if (HitEffect)
		{
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),
				HitEffect,
				GetActorLocation(), // 총알이 부딪힌 현재 위치
				GetActorRotation()  // 터지는 방향
			);
		}
        
		// 펑 터졌으니 총알은 파괴!
		Destroy(); 
	}
}



