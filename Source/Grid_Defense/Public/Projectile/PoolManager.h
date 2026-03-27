
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PoolManager.generated.h"

USTRUCT()
struct FPoolArray
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<AActor*> PooledActors;
};
UCLASS()
class GRID_DEFENSE_API APoolManager : public AActor
{
	GENERATED_BODY()
	
public:	
	APoolManager();

	UFUNCTION(BlueprintCallable, Category = "PoolManager")
	AActor* GetFromPool(TSubclassOf<AActor> PoolClass, FVector Location, FRotator Rotation);

	UFUNCTION(BlueprintCallable, Category = "PoolManager")
	void ReturnToPool(AActor* ActorToReturn);

private:

	UPROPERTY()
	TMap<UClass*, FPoolArray> ObjectPool;

	UPROPERTY(EditAnywhere, Category = "PoolManager")
	int32 PoolSize = 20;
	
};
