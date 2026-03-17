
#pragma once

#include "CoreMinimal.h"
#include "Projectile/ProjectileBase.h"
#include "MagicProjectile.generated.h"

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API AMagicProjectile : public AProjectileBase
{
	GENERATED_BODY()

public:
	AMagicProjectile();

	virtual void FireAtTarget(AActor* TargetActor) override;
};
