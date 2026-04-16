

#include "AnimNotify/AnimNotify_EndFire.h"

#include "Enemy/Boss.h"

void UAnimNotify_EndFire::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                 const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		if (ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner()))
		{
			Boss->EndFireBreath();            
          
			UE_LOG(LogTemp, Warning, TEXT("종료 노티파이 작동! 보스가 다시 걷기 시작합니다."));
		}
	}
}
