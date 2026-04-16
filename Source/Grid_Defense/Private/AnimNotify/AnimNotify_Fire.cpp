

#include "AnimNotify/AnimNotify_Fire.h"

#include "Enemy/Boss.h"

void UAnimNotify_Fire::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                              const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (MeshComp && MeshComp->GetOwner())
	{
		// 2. 주인이 'ABoss' 클래스인지 확인 (우리가 만든 보스 맞나?)
		if (ABoss* Boss = Cast<ABoss>(MeshComp->GetOwner()))
		{
			// 3. 맞다면 보스의 불 뿜기 함수를 다이렉트로 실행!
			Boss->FireBreath();            
			// 디버그용 로그
			UE_LOG(LogTemp, Warning, TEXT("C++ 커스텀 노티파이 작동! 보스가 불을 뿜습니다."));
		}
	}
}
