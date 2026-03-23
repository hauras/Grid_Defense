#include "Projectile/SplashProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h" // 디버그용 구체를 그리기 위해 추가

ASplashProjectile::ASplashProjectile()
{
	SplashRadius = 0.0f;
	
}

void ASplashProjectile::InitSplash(float InRadius, float InDamage)
{
	SplashRadius = InRadius;
	ProjectileDamage = InDamage;
    
	
}

void ASplashProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							  FVector NormalImpulse, const FHitResult& Hit)
{
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);         
	if (GetOwner()) IgnoreActors.Add(GetOwner()); 

	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		ProjectileDamage,     
		GetActorLocation(),   
		SplashRadius,         
		UDamageType::StaticClass(),
		IgnoreActors,         
		this,                  
		GetInstigatorController(),
		true                   
	);

	DrawDebugSphere(GetWorld(), GetActorLocation(), SplashRadius, 12, FColor::Red, false, 1.0f);

	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			HitEffect, 
			GetActorLocation(), 
			GetActorRotation()
		);
	}

	Destroy();
}