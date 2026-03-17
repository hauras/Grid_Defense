

#include "Projectile/MagicProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"


AMagicProjectile::AMagicProjectile()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = true; // 유도탄 켜기!
		ProjectileMovement->HomingAccelerationMagnitude = 2500.f; // 쫓아가는 힘 (수치가 클수록 맹렬하게 꺾임)
	}
}

void AMagicProjectile::FireAtTarget(AActor* TargetActor)
{
	if (TargetActor && ProjectileMovement)
	{
		ProjectileMovement->HomingTargetComponent = TargetActor->GetRootComponent();
        
		Super::FireAtTarget(TargetActor);
	}
}
