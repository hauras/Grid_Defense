
#include "Enemy/EnemyBase.h"

#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/EnemyHPWidget.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = false;

	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
	HPBarWidget->SetupAttachment(RootComponent);
	HPBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); // 머리 위로 100만큼 올리기
	HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeStats();

	// 1. 위젯 컴포넌트에서 실제 위젯 객체 가져오기
	if (HPBarWidget)
	{
		UEnemyHPWidget* HPWidget = Cast<UEnemyHPWidget>(HPBarWidget->GetUserWidgetObject());
		if (HPWidget)
		{
			// 2. 캐릭터의 델리게이트와 위젯의 함수를 연결 (Binding)
			OnHPChanged.AddDynamic(HPWidget, &UEnemyHPWidget::OnHPChanged);
            
			// 3. 초기화 (위젯이 생성될 때 현재 체력을 한 번 전달)
			HPWidget->UpdateHP(CurrentHP, MaxHP);
		}
	}
}

// 몬스터 스탯 적용
void AEnemyBase::InitializeStats()
{
	if (EnemyDataTable && !EnemyDataRowName.IsNone())
	{
		FEnemyData* Data = EnemyDataTable->FindRow<FEnemyData>(EnemyDataRowName, TEXT(""));
		if (Data)
		{
			MaxHP = Data->MaxHP;       // 데이터 테이블의 100, 200, 500 등 드래곤 종류에 맞는 체력 세팅!
			CurrentHP = MaxHP;

			GetCharacterMovement()->MaxWalkSpeed = Data->MoveSpeed;

			if (Data->EnemyMesh)
			{
				GetMesh()->SetSkeletalMesh(Data->EnemyMesh);
			}
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
}

// 데미지 적용
float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                             class AController* EventInstigator, AActor* DamageCauser)
{
	if (bIsDead) return 0.f;
	
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHP -= ActualDamage;
	UE_LOG(LogTemp, Warning, TEXT("[%s] 남은 체력: %f"), *GetName(), CurrentHP);

	OnHPChanged.Broadcast(CurrentHP, MaxHP);
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

	if (HPBarWidget)
	{
		HPBarWidget->SetVisibility(false);
	}
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->StopMovementImmediately();

	float DestroyTime = 0.1f; 

	if (DeathMontage)
	{
		DestroyTime = PlayAnimMontage(DeathMontage);
	}

	SetLifeSpan(DestroyTime - 0.3f); 
}





