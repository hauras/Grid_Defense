#include "Projectile/SplashProjectile.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "DrawDebugHelpers.h" // 디버그용 구체를 그리기 위해 추가

ASplashProjectile::ASplashProjectile()
{
	SplashRadius = 0.0f; // 초기값은 0으로 설정 (InitSplash에서 채워짐)
}

void ASplashProjectile::InitSplash(float InRadius, float InDamage)
{
	SplashRadius = InRadius;
	ProjectileDamage = InDamage;
    
	// 💡 팁: 나중에 폭발 범위가 커지면 총알의 외형(Scale)도 
	// 여기서 비례해서 키워주는 로직을 넣으면 비주얼이 더 좋아집니다.
}

void ASplashProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
							  FVector NormalImpulse, const FHitResult& Hit)
{
	// 1. 데미지 무시 목록 작성
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);            // 자기 자신 제외
	if (GetOwner()) IgnoreActors.Add(GetOwner()); // 발사한 타워 제외

	// 2. 광역 데미지 적용
	UGameplayStatics::ApplyRadialDamage(
		GetWorld(),
		ProjectileDamage,       // 데미지 양
		GetActorLocation(),     // 폭발 위치
		SplashRadius,           // 폭발 반지름
		UDamageType::StaticClass(),
		IgnoreActors,           // 무시할 액터들
		this,                   // 데미지 원인 액터
		GetInstigatorController(), // 데미지를 시킨 컨트롤러
		true                    // bDoFullDamage: true면 범위 안 모든 적에게 100% 데미지 (디펜스 정석)
	);

	// 3. 비주얼 디버깅 (폭발 범위를 에디터에서 빨간 구체로 확인)
	DrawDebugSphere(GetWorld(), GetActorLocation(), SplashRadius, 12, FColor::Red, false, 1.0f);

	// 4. 이펙트 재생
	if (HitEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(), 
			HitEffect, 
			GetActorLocation(), 
			GetActorRotation()
		);
	}

	// 5. 투사체 파괴
	Destroy();
}