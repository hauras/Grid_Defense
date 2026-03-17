

#include "Projectile/ProjectileBase.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

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
		UE_LOG(LogTemp, Warning, TEXT("기본 발사체가 [%s]에 적중했습니다!"), *OtherActor->GetName());
        
		UGameplayStatics::ApplyDamage(
			OtherActor,                 // 데미지를 받을 대상 (드래곤)
			ProjectileDamage,           // 타워에서 받아온 그 데미지 수치!
			nullptr,                    // 가해자의 컨트롤러 (일단 널로 두어도 됨)
			this,                       // 가해자 액터 (이 총알)
			UDamageType::StaticClass()  // 기본 데미지 타입
		);
        
		// 펑 터졌으니 총알은 파괴!
		Destroy(); // 적중 시 파괴
	}
}

void AProjectileBase::BeginPlay()
{
	Super::BeginPlay();

	if (GetOwner())
	{
		// 내 충돌체(Collision)가 날아갈 때 주인은 그냥 무시하고 통과해라!
		Collision->IgnoreActorWhenMoving(GetOwner(), true);
	}
}

