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

	
	InputComponent->BindKey(EKeys::One, IE_Pressed, this, &AGridController::OnKey1Pressed);
	InputComponent->BindKey(EKeys::Two, IE_Pressed, this, &AGridController::OnKey2Pressed);
	InputComponent->BindKey(EKeys::Three, IE_Pressed, this, &AGridController::OnKey3Pressed);
	InputComponent->BindKey(EKeys::Four, IE_Pressed, this, &AGridController::OnKey4Pressed);

}

void AGridController::CursorTrace()
{
	if (!GridManager || !SelectedTowerData || !bBuildModeActive) 
	{
		if (CurrentPreviewActor) CurrentPreviewActor->SetActorHiddenInGame(true);
		return;
	}

	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
	{
		float TileSize = GridManager->GetTileSize(); 
		FVector GridCenter = GridManager->GetActorLocation() + 
						FVector(GridX * TileSize, GridY * TileSize, 50.0f);

		if (!CurrentPreviewActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
       
			CurrentPreviewActor = GetWorld()->SpawnActor<ATowerBase>(SelectedTowerData->TowerActorClass, GridCenter, FRotator::ZeroRotator, SpawnParams);
       
			if (CurrentPreviewActor)
			{
				CurrentPreviewActor->InitTower(SelectedTowerData, true); 
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
	// 💡 SelectedTowerData가 있는지 확인
	if (!SelectedTowerData || !bBuildModeActive) return;

	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
	{
		// 💡 SelectedTowerData를 전달
		GridManager->AddTower(GridX, GridY, SelectedTowerData);
	}
}

void AGridController::SetSelectedTower(UTowerData* NewData)
{
	if (!NewData) return;

	SelectedTowerData = NewData;

	// 타워가 바뀌었으므로 기존 프리뷰는 제거 (그래야 새 모양으로 생성됨)
	if (CurrentPreviewActor)
	{
		CurrentPreviewActor->Destroy();
		CurrentPreviewActor = nullptr;
	}
}

void AGridController::OnKey1Pressed()
{
	if (TowerData.IsValidIndex(0)) 
	{
		SetSelectedTower(TowerData[0]);
	}
}

void AGridController::OnKey2Pressed()
{
	if (TowerData.IsValidIndex(1)) 
	{
		SetSelectedTower(TowerData[1]);
	}
}

void AGridController::OnKey3Pressed()
{
	if (TowerData.IsValidIndex(2)) 
	{
		SetSelectedTower(TowerData[2]);
	}
}

void AGridController::OnKey4Pressed()
{
	if (TowerData.IsValidIndex(3)) 
	{
		SetSelectedTower(TowerData[3]);
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
	if (!CurrentPreviewActor || !SelectedTowerData) return;

	UStaticMeshComponent* MeshComp = CurrentPreviewActor->FindComponentByClass<UStaticMeshComponent>();
	
	if (MeshComp && SelectedTowerData->PreviewMesh)
	{
		if (MeshComp->GetStaticMesh() != SelectedTowerData->PreviewMesh)
		{
			MeshComp->SetStaticMesh(SelectedTowerData->PreviewMesh);
		}
	}
}