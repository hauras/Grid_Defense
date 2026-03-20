#include "Controller/GridController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "Data/TowerData.h"
#include "Tower/TowerBase.h"

AGridController::AGridController()
{
	bShowMouseCursor = true;
	bEnableClickEvents = false;
	bEnableMouseOverEvents = false;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
}

void AGridController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly InputModeData; 
	SetInputMode(InputModeData);
	bShowMouseCursor = true;

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void AGridController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
}

void AGridController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent))
	{
		EnhancedInputComponent->BindAction(ClickAction, ETriggerEvent::Started, this, &AGridController::OnMouseClick);
	}
}

void AGridController::CursorTrace()
{
	if (!GridManager || !TowerData || !bBuildModeActive) 
	{
		if (CurrentPreviewActor) CurrentPreviewActor->SetActorHiddenInGame(true);
		return;
	}

	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
	{
		// 💡 수정 1: 직접 변수에 접근하지 말고 Getter 함수를 호출하세요!
		float TileSize = GridManager->GetTileSize(); 
    
		// 💡 수정 2: 격자 중심 좌표 계산
		// (보통 그리드 매니저의 위치가 0,0 타일의 중심이라고 가정할 때의 계산식입니다)
		FVector GridCenter = GridManager->GetActorLocation() + 
						FVector(GridX * TileSize, GridY * TileSize, 50.0f);

		if (!CurrentPreviewActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
       
			// 타워 데이터에서 클래스 정보 가져와서 스폰
			CurrentPreviewActor = GetWorld()->SpawnActor<ATowerBase>(TowerData->TowerActorClass, GridCenter, FRotator::ZeroRotator, SpawnParams);
       
			if (CurrentPreviewActor)
			{
				CurrentPreviewActor->InitTower(TowerData, true); // 유령 모드로 초기화
			}
		}
		else
		{
			CurrentPreviewActor->SetActorHiddenInGame(false);
			CurrentPreviewActor->SetActorLocation(GridCenter);
			UpdateGhostVisual();
		}
	}
	else
	{
		if (CurrentPreviewActor) CurrentPreviewActor->SetActorHiddenInGame(true);
	}
}

void AGridController::OnMouseClick()
{
	if (!TowerData || !bBuildModeActive) return;

	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
	{
		GridManager->AddTower(GridX, GridY, TowerData);
	}
}

bool AGridController::GetGridLocationUnderCursor(int32& OutX, int32& OutY)
{
	if (!GridManager) return false;

	FHitResult CursorHit;
	if (GetHitResultUnderCursor(ECC_Visibility, true, CursorHit))
	{
		// 1. 그리드 매니저의 위치로부터 얼마나 떨어졌나 계산
		FVector RelativeLocation = CursorHit.ImpactPoint - GridManager->GetActorLocation();
       
		// 💡 수정 포인트: 변수에 직접 접근하지 말고 Getter 함수를 호출하세요!
		float TileSize = GridManager->GetTileSize(); // 이 Getter를 GridManager에 추가해야겠네요!
		float HalfTile = TileSize * 0.5f;

		// 2. 상대 좌표를 타일 인덱스(X, Y)로 변환
		OutX = FMath::FloorToInt((RelativeLocation.X + HalfTile) / TileSize);
		OutY = FMath::FloorToInt((RelativeLocation.Y + HalfTile) / TileSize);

		// 💡 수정 포인트: Width와 Height도 Getter를 통해서 가져옵니다!
		return (OutX >= 0 && OutX < GridManager->GetGridWidth() && 
				OutY >= 0 && OutY < GridManager->GetGridHeight());
	}
	return false;
}

void AGridController::UpdateGhostVisual()
{
	if (!CurrentPreviewActor || !TowerData) return;
	UStaticMeshComponent* MeshComp = CurrentPreviewActor->FindComponentByClass<UStaticMeshComponent>();
	if (MeshComp && TowerData->PreviewMesh)
	{
		if (MeshComp->GetStaticMesh() != TowerData->PreviewMesh)
		{
			MeshComp->SetStaticMesh(TowerData->PreviewMesh);
		}
	}
}