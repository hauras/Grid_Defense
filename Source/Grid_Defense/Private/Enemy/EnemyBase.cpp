// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/EnemyBase.h"

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

float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
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
	UE_LOG(LogTemp, Error, TEXT("[%s] 사망했습니다!"), *GetName());
	// 일단 바로 파괴 (나중에 애니메이션이나 보상 처리 추가)
	Destroy();
}





