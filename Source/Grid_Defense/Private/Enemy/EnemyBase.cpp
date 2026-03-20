
#include "Enemy/EnemyBase.h"

#include "AIController.h"
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
}

void AEnemyBase::InitializeEnemy(FName InRowName)
{
	EnemyDataRowName = InRowName; // 내 이름표 내가 달기
	InitializeStats();
}

void AEnemyBase::BeginPlay()
{
	Super::BeginPlay();
	InitializeStats();

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

	if (bIsDead || Waypoints.Num() == 0) return;

	FVector TargetLocation = Waypoints[CurrentIndex];

	FVector MyLocation = GetActorLocation();
	TargetLocation.Z = MyLocation.Z;

	float DistanceToTarget = FVector::Distance(TargetLocation, MyLocation);
	if (DistanceToTarget < 10.0f)
	{
		CurrentIndex++;

		if (CurrentIndex >= Waypoints.Num())
		{
			Waypoints.Empty();
			return;
		}
	}
	else
	{
		FVector Direction = (TargetLocation - MyLocation).GetSafeNormal();
		AddMovementInput(Direction, 1.0f);
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

void AEnemyBase::RecalculatePath()
{
	// 1. GridManager 찾기
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (!GridManager) return;

	// 2. 내 현재 위치를 가져오기
	FVector MyLoc = GetActorLocation();
    
	// 그리드 매니저의 시작점으로부터 상대적인 거리 계산
	FVector RelativeLoc = MyLoc - GridManager->GetActorLocation();
	float TileSize = GridManager->GetTileSize();

	// 💡 [중요] MyTile 변수를 선언함과 동시에 계산된 값을 넣어줍니다 (C4700 해결!)
	FIntPoint MyTile;
	MyTile.X = FMath::FloorToInt((RelativeLoc.X + (TileSize * 0.5f)) / TileSize);
	MyTile.Y = FMath::FloorToInt((RelativeLoc.Y + (TileSize * 0.5f)) / TileSize);

	// 3. 목적지(넥서스) 좌표 설정
	int32 EndX = GridManager->GetGridWidth() - 1;
	int32 EndY = GridManager->GetGridHeight() - 1;

	// 4. 새로운 길 찾기 수행
	TArray<FIntPoint> NewGridPath;
    
	// 내 현재 타일 위치(MyTile)에서 넥서스까지 다시 길을 찾습니다.
	if (GridManager->FindPath(MyTile.X, MyTile.Y, EndX, EndY, NewGridPath))
	{
		TArray<FVector> NewWorldPath;
		for (FIntPoint Node : NewGridPath)
		{
			FVector Pos = GridManager->GetTileWorldPosition(Node.X, Node.Y);
			// 몬스터가 공중에 뜨거나 땅에 박히지 않게 현재 높이(Z) 유지
			Pos.Z = MyLoc.Z; 
			NewWorldPath.Add(Pos);
		}

		// 5. 새 수첩으로 교체!
		SetPath(NewWorldPath);
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





