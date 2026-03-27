
#include "Enemy/EnemyBase.h"

#include "AIController.h"
#include "GridGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameMode/GridGameMode.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Widget/EnemyHPWidget.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
	HPBarWidget->SetupAttachment(RootComponent);
	HPBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); // 머리 위로 100만큼 올리기
	HPBarWidget->SetWidgetSpace(EWidgetSpace::Screen);
	if (GetCapsuleComponent())
	{
		GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
}

void AEnemyBase::InitializeEnemy(FName InRowName)
{
	EnemyDataRowName = InRowName; 
	InitializeStats();
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeStats();

	CachedGridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (HPBarWidget)
	{
		UEnemyHPWidget* HPWidget = Cast<UEnemyHPWidget>(HPBarWidget->GetUserWidgetObject());
		if (HPWidget)
		{
			OnHPChanged.AddDynamic(HPWidget, &UEnemyHPWidget::OnHPChanged);
            
			HPWidget->UpdateHP(CurrentHP, MaxHP);
		}
	}
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead || !CachedGridManager) return;

	// 1. 현재 내 위치의 화살표 방향을 물어본다
	FVector MoveDirection = CachedGridManager->GetFlowDirection(GetActorLocation());

	// 2. 화살표가 있다면 그 방향으로 이동!
	if (!MoveDirection.IsZero())
	{
		AddMovementInput(MoveDirection, 1.0f);

		// 몬스터가 이동 방향을 부드럽게 바라보게 하고 싶다면?
		FRotator TargetRot = MoveDirection.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.f));
	}
	else
	{
		// 💡 방향이 ZeroVector라는 건 넥서스에 도착했다는 뜻일 확률이 높음!
		// 여기서 넥서스 공격 로직이나 파괴 로직을 넣으면 됩니다.
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
			MaxHP = Data->MaxHP;       
			CurrentHP = MaxHP;

			GetCharacterMovement()->MaxWalkSpeed = Data->MoveSpeed;

			BaseMoveSpeed = Data->MoveSpeed; 

			if (Data->EnemyMesh)
			{
				GetMesh()->SetSkeletalMesh(Data->EnemyMesh);
			}
		}
	}
}

void AEnemyBase::SetPath(const TArray<FVector>& NewPath)
{
	Waypoints = NewPath;
	CurrentIndex = 0;
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

	OnEnemyDied.Broadcast();
	
	AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->AddGold(10);
	}
	SetActorEnableCollision(false);
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

void AEnemyBase::ApplySlow(float SlowDuration)
{
	if (!GameplayTags.HasTagExact(FGridGameplayTags::Get().State_Slow))
	{
		GameplayTags.AddTag(FGridGameplayTags::Get().State_Slow);

		GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed * 0.5f;
	}
	GetWorldTimerManager().SetTimer(SlowTimerHandle, this, &AEnemyBase::RemoveSlow, SlowDuration, false);
}

void AEnemyBase::RemoveSlow()
{
	GameplayTags.RemoveTag(FGridGameplayTags::Get().State_Slow);

	GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;
}





