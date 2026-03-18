
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProjectileBase.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
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

	virtual void FireAtTarget(AActor* TargetActor);

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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UNiagaraSystem> Projectile;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	TObjectPtr<UNiagaraComponent> ProjectileComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
	TObjectPtr<UNiagaraSystem> HitEffect;
};
