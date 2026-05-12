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
#include "Enemy/EnemySpawner.h"
#include "Data/ElementalData.h"

AEnemyBase::AEnemyBase()
{
	PrimaryActorTick.bCanEverTick = true;

	HPBarWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("HPBarWidget"));
	HPBarWidget->SetupAttachment(RootComponent);
	HPBarWidget->SetRelativeLocation(FVector(0.f, 0.f, 100.f)); 
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

	if (bIsDead || !CachedGridManager || bIsAttackingNexus) return;

	FVector MoveDirection = CachedGridManager->GetFlowDirection(GetActorLocation());

	if (!MoveDirection.IsZero())
	{
		AddMovementInput(MoveDirection, 1.0f);

		FRotator TargetRot = MoveDirection.Rotation();
		SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRot, DeltaTime, 5.f));
	}
}

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
			MyLifeDamage = Data->LifeDamage; 
			this->EnemyTags = Data->EnemyTags;
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

	FGameplayTag AttackTag = FGameplayTag::EmptyTag;

	if (AProjectileBase* Projectile = Cast<AProjectileBase>(DamageCauser))
	{
		AttackTag = Projectile->DamageTag;
	}
	else if (ATowerBase* Tower = Cast<ATowerBase>(DamageCauser))
	{
		AttackTag = Tower->TowerDamageTag;
	}

	float DamageMultiplier = 1.0f;
	FLinearColor TextColor = FLinearColor::White;

	// [수정] 하드코딩 제거 → ElementalMatchupTable 참조
	if (AttackTag.IsValid() && ElementalMatchupTable)
	{
		TArray<FElementalMatchup*> AllRows;
		ElementalMatchupTable->GetAllRows<FElementalMatchup>(TEXT(""), AllRows);

		for (FElementalMatchup* Row : AllRows)
		{
			if (Row->AttackTag == AttackTag && EnemyTags.HasTagExact(Row->EnemyTag))
			{
				DamageMultiplier = Row->DamageMultiplier;
				TextColor = Row->TextColor;
				break;
			}
		}
	}

	// 역상성이면 회색
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

	GetWorldTimerManager().ClearTimer(DamageTextTimerHandle);
	GetWorldTimerManager().SetTimer(DamageTextTimerHandle, this, &AEnemyBase::ResetDamageText, 1.5f, false);
	
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

	GetWorldTimerManager().ClearTimer(NexusAttackTimerHandle);
	
	if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass())))
	{
		Spawner->OnEnemyDefeated();
	}

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

		if (UEnemyHPWidget* HPWidget = Cast<UEnemyHPWidget>(HPBarWidget->GetUserWidgetObject()))
		{
			HPWidget->SetSlowVisible(true);
		}
	}
	GetWorldTimerManager().SetTimer(SlowTimerHandle, this, &AEnemyBase::RemoveSlow, SlowDuration, false);
}

void AEnemyBase::RemoveSlow()
{
	GameplayTags.RemoveTag(FGridGameplayTags::Get().State_Slow);
	GetCharacterMovement()->MaxWalkSpeed = BaseMoveSpeed;

	if (UEnemyHPWidget* HPWidget = Cast<UEnemyHPWidget>(HPBarWidget->GetUserWidgetObject()))
	{
		HPWidget->SetSlowVisible(false);
	}
}

void AEnemyBase::AttackNexus()
{
	if (bIsDead) return;

	if (AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		GM->DecreaseLife(MyLifeDamage);
	}
}

void AEnemyBase::ReachNexus()
{
	if (bIsDead || bIsAttackingNexus) return;
    
	bIsAttackingNexus = true;
	GetCharacterMovement()->StopMovementImmediately();
	AttackNexus();
	GetWorldTimerManager().SetTimer(NexusAttackTimerHandle, this, &AEnemyBase::AttackNexus, 1.0f, true);
}
