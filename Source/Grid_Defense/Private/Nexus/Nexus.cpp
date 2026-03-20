

#include "Nexus/Nexus.h"

#include "Components/BoxComponent.h"
#include "Enemy/EnemyBase.h"
#include "GameMode/GridGameMode.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ANexus::ANexus()
{
	PrimaryActorTick.bCanEverTick = false;

	Collision = CreateDefaultSubobject<UBoxComponent>(FName("Collision"));
	RootComponent = Collision;
	Collision->SetBoxExtent(FVector(100.f, 100.f, 100.f));
	Collision->SetCollisionProfileName(FName("OverlapAllDynamic"));

	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(FName("StaticMesh"));
	StaticMesh->SetupAttachment(RootComponent);

	Collision->OnComponentBeginOverlap.AddDynamic(this, &ANexus::OnNexusOverlap);
}	

void ANexus::OnNexusOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor != this)
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (Enemy)
		{
			AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (GM)
			{
				GM->DecreaseLife(1);
			}

			Enemy->Destroy();
		}
	}
}


