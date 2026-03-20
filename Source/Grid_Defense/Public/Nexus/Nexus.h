
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Nexus.generated.h"

class UBoxComponent;

UCLASS()
class GRID_DEFENSE_API ANexus : public AActor
{
	GENERATED_BODY()
	
public:	
	ANexus();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> Collision;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> StaticMesh;

	UFUNCTION()
	void OnNexusOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};
