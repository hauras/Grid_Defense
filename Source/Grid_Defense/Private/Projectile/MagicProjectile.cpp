

#include "Projectile/MagicProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"


AMagicProjectile::AMagicProjectile()
{
	if (ProjectileMovement)
	{
		ProjectileMovement->bIsHomingProjectile = true;
		ProjectileMovement->HomingAccelerationMagnitude = 7000.f;
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
