#pragma once
 
#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "SplashProjectile.generated.h"
 
UCLASS()
class GRID_DEFENSE_API ASplashProjectile : public AProjectileBase
{
	GENERATED_BODY()
 
public:
	ASplashProjectile();
 
	void InitSplash(float InRadius, float InDamage);
 
protected:
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
				   FVector NormalImpulse, const FHitResult& Hit) override;
 
private:
	// [수정] VisibleAnywhere로 노출 — 에디터에서 값 확인 가능
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	float SplashRadius = 0.0f;
};