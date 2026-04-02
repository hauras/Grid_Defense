
#include "Enemy/EnemyBase.h"

#include "AIController.h"
#include "GridGameplayTags.h"
#include "Components/CapsuleComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameMode/GridGameMode.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "Projectile/ProjectileBase.h"
#include "Tower/TowerBase.h"
#include "UI/Widget/EnemyHPWidget.h"
#include "UI/DamageText/DamageTextComponent.h"
#include "TimerManager.h"

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

	DamageTextComp = CreateDefaultSubobject<UDamageTextComponent>(TEXT("DamageTextComp"));
	DamageTextComp->SetupAttachment(RootComponent);
	DamageTextComp->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); 
	DamageTextComp->SetWidgetSpace(EWidgetSpace::Screen);
	DamageTextComp->SetDrawSize(FVector2D(200.f, 50.f));
}

void AEnemyBase::InitializeEnemy(FName InRowName)
{
	EnemyDataRowName = InRowName; 
	InitializeStats();
}

void AEnemyBase::ResetDamageText()
{
	AccumulatedDamage = 0.0f;
	if (DamageTextComp)
	{
		DamageTextComp->HideDamageText();
	}
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

	ResetDamageText();
}

void AEnemyBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsDead || !CachedGridManager) return;

	FVector MoveDirection = CachedGridManager->GetFlowDirection(GetActorLocation());

	if (!MoveDirection.IsZero())
	{
		AddMovementInput(MoveDirection, 1.0f);

		FRotator TargetRot = MoveDirection.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.f));
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
			MyGoldReward = Data->GoldReward;
			this->EnemyTags = Data->EnemyTags;
			
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

float AEnemyBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
                       class AController* EventInstigator, AActor* DamageCauser)
{
    if (bIsDead) return 0.f;

    const FGridGameplayTags& GridTags = FGridGameplayTags::Get();
    FGameplayTag AttackTag = FGameplayTag::EmptyTag;

    if (AProjectileBase* Projectile = Cast<AProjectileBase>(DamageCauser))
    {
       AttackTag = Projectile->DamageTag;
    }
    else if (ATowerBase* Tower = Cast<ATowerBase>(DamageCauser))
    {
       AttackTag = Tower->TowerDamageTag;
    }

    // ==========================================
    // 🌟 [수정된 부분] 상성에 따른 데미지 배율 및 색상 결정
    // ==========================================
    float DamageMultiplier = 1.0f;
    FLinearColor TextColor = FLinearColor::White; // 기본 색상 (비상성)

    if (AttackTag.IsValid())
    {
       // 🔥 자연 속성 몬스터가 불 공격을 받았을 때 (1.5배)
       if (EnemyTags.HasTagExact(GridTags.Enemy_Nature) && AttackTag.MatchesTagExact(GridTags.Damage_Fire))
       {
          DamageMultiplier = 1.5f;
          TextColor = FLinearColor::Red; // 불 약점: 빨간색
       }
       // ⚡ 물 속성 몬스터가 전기 공격(라이트닝 타워)을 받았을 때 (2.0배)
       else if (EnemyTags.HasTagExact(GridTags.Enemy_Water) && AttackTag.MatchesTagExact(GridTags.Damage_Lightning))
       {
          DamageMultiplier = 2.0f;
          TextColor = FLinearColor::Yellow; // 전기 약점: 노란색
       }
    }

    // 공격 반감(역상성)이 있다면 회색으로 처리
    if (DamageMultiplier < 1.0f)
    {
        TextColor = FLinearColor::Gray; 
    }

    float FinalDamage = DamageAmount * DamageMultiplier;
    
    AccumulatedDamage += FinalDamage;

    if (DamageTextComp)
    {
        DamageTextComp->SetDamageText(AccumulatedDamage, TextColor);
    }

    // 3. 타이머 갱신 (1.5초 동안 안 맞으면 누적 초기화 및 글자 숨김)
    GetWorldTimerManager().ClearTimer(DamageTextTimerHandle);
    GetWorldTimerManager().SetTimer(DamageTextTimerHandle, this, &AEnemyBase::ResetDamageText, 1.5f, false);
    
    // ==========================================

    CurrentHP -= FinalDamage;

    OnHPChanged.Broadcast(CurrentHP, MaxHP);
    if (CurrentHP <= 0.f)
    {
       Die();
    }

    return FinalDamage;
}

void AEnemyBase::Die()
{
	if (bIsDead) return; 
	bIsDead = true;      

	OnEnemyDied.Broadcast();
	
	AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{
		GM->AddGold(MyGoldReward);
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





