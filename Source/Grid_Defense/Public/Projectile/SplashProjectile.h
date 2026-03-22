#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h" // 부모 클래스 포함
#include "SplashProjectile.generated.h"

UCLASS()
class GRID_DEFENSE_API ASplashProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	ASplashProjectile();

	// 💡 추가: 타워에서 발사할 때 이 함수를 통해 값 주입
	void InitSplash(float InRadius, float InDamage);

protected:
	// 기존 OnHit을 광역용으로 덮어쓰기(Override)
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
					   FVector NormalImpulse, const FHitResult& Hit) override;

private:
	// 이 투사체만의 폭발 범위
	float SplashRadius;
};