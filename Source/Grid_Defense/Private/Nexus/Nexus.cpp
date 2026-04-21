

#include "Nexus/Nexus.h"

#include "Components/BoxComponent.h"
#include "Enemy/EnemyBase.h"
#include "GameMode/GridGameMode.h"
#include "Kismet/GameplayStatics.h"

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
       
		if (Enemy && !Enemy->IsDead()) 
		{
			AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			if (GM)
			{
				// 💡 여기서 에러가 나지 않고 각 몬스터 고유의 데미지를 깎습니다!
				int32 DamageAmount = Enemy->GetLifeDamage(); 
				GM->DecreaseLife(DamageAmount);
			}

			// 💡 돈을 주지 않고 스포너 카운트만 내린 뒤 파괴!
			Enemy->ReachNexus(); 
		}
	}
}


