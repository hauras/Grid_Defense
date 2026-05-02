#include "Projectile/SplashProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
 
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
 
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			HitEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
 
	// [수정] Destroy() 대신 풀에 반환
	ReturnToManager();
}