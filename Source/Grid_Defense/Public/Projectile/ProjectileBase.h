
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class UProjectileMovementComponent;
class USphereComponent;

UCLASS()
class GRID_DEFENSE_API AProjectileBase : public AActor
{
	GENERATED_BODY()
	
public:	
	AProjectileBase();

	void SetDamage(float Damage);

	virtual void FireInDirection(const FVector& ShootDirection);

	// 💡 타겟을 지정하는 발사 함수 (자식인 유도탄 클래스에서 오버라이드할 예정!)
	virtual void FireAtTarget(AActor* TargetActor);

	// 💡 충돌 시 호출될 가상 함수 (나중에 폭발 이펙트 등을 자식마다 다르게 구현 가능)
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);


protected:

	virtual void BeginPlay() override;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USphereComponent> Collision;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	float ProjectileDamage;

	
};
