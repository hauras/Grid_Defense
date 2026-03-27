

#include "Projectile/PoolManager.h"

APoolManager::APoolManager()
{
	PrimaryActorTick.bCanEverTick = false;

}

AActor* APoolManager::GetFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation)
{
	if (!PoolClass) return nullptr;

	AActor* SpawnedActor = nullptr;

	if (ObjectPool.Contains(PoolClass) && ObjectPool[PoolClass].PooledActors.Num() > 0)
	{
		SpawnedActor = ObjectPool[PoolClass].PooledActors.Pop();
		SpawnedActor->SetActorLocationAndRotation(Location, Rotation);
	}
	else
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		
		SpawnedActor = GetWorld()->SpawnActor<AActor>(PoolClass, Location, Rotation, SpawnParams);
	}

	if (SpawnedActor)
	{
		SpawnedActor->SetActorHiddenInGame(false);
		SpawnedActor->SetActorEnableCollision(true);
		SpawnedActor->SetActorTickEnabled(true);
	}

	return SpawnedActor;
}

void APoolManager::ReturnToPool(AActor* ActorToReturn)
{
	if (!ActorToReturn) return;

	ActorToReturn->SetActorHiddenInGame(true);    
	ActorToReturn->SetActorEnableCollision(false); 
	ActorToReturn->SetActorTickEnabled(false);

	UClass* Class = ActorToReturn->GetClass();

	
	ObjectPool.FindOrAdd(Class).PooledActors.AddUnique(ActorToReturn);
}

