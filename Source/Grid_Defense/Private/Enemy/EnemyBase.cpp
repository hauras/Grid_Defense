
#include "Enemy/EnemyBase.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();

	InitializeStats();
}

// 몬스터 스탯 적용
void AEnemyBase::InitializeStats()
{
	if (EnemyDataTable && !EnemyDataRowName.IsNone())
	{
		FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(EnemyDataRowName, TEXT(""));
		if (Data)
		{
			CurrentHP = Data->MaxHP;

			GetCharacterMovement()->MaxWalkSpeed = Data->MoveSpeed;

			if (Data->EnemyMesh)
			{
				GetMesh()->SetSkeletalMesh(Data->EnemyMesh);
			}
			UE_LOG(LogTemp, Warning, TEXT("[%s] 로드 완료! HP: %f"), *EnemyDataRowName.ToString(), CurrentHP);
		}
	}
}

// 길 찾기
void AEnemyBase::MoveToTarget(FVector TargetLocation)
{
	AAIController* AIC = Cast<AAIController>(GetController());
	if (AIC)
	{
		AIC->MoveToLocation(TargetLocation);
	}
	UE_LOG(LogTemp, Warning, TEXT("목적지로 이동 시도: %s"), *TargetLocation.ToString());
}

// 데미지 적용
float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;
	
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP -= ActualDamage;
	UE_LOG(LogTemp, Warning, TEXT("[%s] 남은 체력: %f"), *GetName(), CurrentHP);

	if (CurrentHP <= 0.f)
	{
		Die();
	}

	return ActualDamage;
}

void AEnemyBase::Die()
{
	if (bIsDead) return; 
	bIsDead = true;      

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();

	// 💡 애니메이션 길이를 저장할 변수 (기본값은 혹시 모를 에러 방지용 0.1초)
	float DestroyTime = 0.1f; 

	if (DeathMontage)
	{
		// PlayAnimMontage는 재생된 몽타주의 길이를 반환합니다!
		DestroyTime = PlayAnimMontage(DeathMontage);
	}

	// 💡 3.0f 대신 애니메이션 길이를 그대로 넣어줍니다.
	SetLifeSpan(DestroyTime - 0.3f); 
}





