#include "Controller/GridController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "Data/TowerData.h"
#include "Tower/TowerBase.h"
#include "Blueprint/UserWidget.h"
#include "Camera/Camera.h"
#include "UI/Widget/TowerSellWidget.h"

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

	FInputModeGameAndUI InputModeData; 
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);

	GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));

	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (DefaultMappingContext)
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	ACamera* Camera = Cast<ACamera>(UGameplayStatics::GetActorOfClass(GetWorld(), ACamera::StaticClass()));
	if (Camera)
	{
		Camera->OnCameraMoved.AddDynamic(this, &AGridController::CloseSellWidget);
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

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(InputComponent);

	EIC->BindAction(ClickAction,         ETriggerEvent::Started, this, &AGridController::OnMouseClick);
	EIC->BindAction(PauseAction,         ETriggerEvent::Triggered, this, &AGridController::TogglePause);
	EIC->BindAction(SelectTower1Action,  ETriggerEvent::Started, this, &AGridController::OnSelectTower1);
	EIC->BindAction(SelectTower2Action,  ETriggerEvent::Started, this, &AGridController::OnSelectTower2);
	EIC->BindAction(SelectTower3Action,  ETriggerEvent::Started, this, &AGridController::OnSelectTower3);
	EIC->BindAction(SelectTower4Action,  ETriggerEvent::Started, this, &AGridController::OnSelectTower4);
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
		int32 Index = GridManager->GetIndex(GridX, GridY);
		const TArray<FGridInfo>& GridArray = GridManager->GetGridArray();
		if (GridArray.IsValidIndex(Index) && GridArray[Index].OccupyingTower)
		{
			if (CurrentPreviewActor) CurrentPreviewActor->SetActorHiddenInGame(true);
			return;
		}
		
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

void AGridController::ShowSellWidget(int32 GridX, int32 GridY)
{
	const TArray<FGridInfo>& GridArray = GridManager->GetGridArray();
	int32 Index = GridManager->GetIndex(GridX, GridY);
	if (!GridArray.IsValidIndex(Index)) return;

	ATowerBase* Tower = GridArray[Index].OccupyingTower;
	if (!Tower) return;

	int32 RefundAmount = Tower->GetTowerData()->BuildCost * 0.5f;

	UTowerSellWidget* SellWidget = CreateWidget<UTowerSellWidget>(this, TowerSellWidgetClass);
	if (SellWidget)
	{
		SellWidget->SetTowerInfo(GridX, GridY, RefundAmount);
		SellWidget->AddToViewport();
		ActiveSellWidget = SellWidget;

		FVector2D ScreenPos;
		if (ProjectWorldLocationToScreen(Tower->GetActorLocation(), ScreenPos))
		{
			SellWidget->SetPositionInViewport(FVector2D(ScreenPos.X + 50.f, ScreenPos.Y - 50.f));
		}
		
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		SetInputMode(InputMode);
	}
}

void AGridController::CloseSellWidget()
{
	if (ActiveSellWidget)
	{
		ActiveSellWidget->RemoveFromParent();
		ActiveSellWidget = nullptr;
	}
}

void AGridController::OnMouseClick()
{
	if (!GridManager) return;

	int32 GridX, GridY;
	if (!GetGridLocationUnderCursor(GridX, GridY)) return;

	const TArray<FGridInfo>& GridArray = GridManager->GetGridArray();
	int32 Index = GridManager->GetIndex(GridX, GridY);
	if (!GridArray.IsValidIndex(Index)) return;

	// 타워가 있는 타일 클릭 → 판매 UI 표시 (빌드 모드 여부 무관)
	if (GridArray[Index].OccupyingTower)
	{
		ShowSellWidget(GridX, GridY);
		return;
	}

	// 빌드 모드이고 타워가 선택된 경우 → 타워 배치
	if (bBuildModeActive && SelectedTowerData)
	{
		GridManager->AddTower(GridX, GridY, SelectedTowerData);
	}
}

void AGridController::SetSelectedTower(UTowerData* NewData)
{
	if (!NewData) return;

	SelectedTowerData = NewData;

	if (CurrentPreviewActor)
	{
		CurrentPreviewActor->Destroy();
		CurrentPreviewActor = nullptr;
	}
}

void AGridController::TogglePause()
{
	if (PauseMenuClass)
	{
		UGameplayStatics::SetGamePaused(this, true);

		bShowMouseCursor = true;
		FInputModeUIOnly InputModeData;
		SetInputMode(InputModeData);

		UUserWidget* PauseMenu = CreateWidget<UUserWidget>(this, PauseMenuClass);
		if (PauseMenu)
		{
			PauseMenu->AddToViewport();
		}
	}
}

void AGridController::OnSelectTower1() { if (TowerData.IsValidIndex(0)) SetSelectedTower(TowerData[0]); }
void AGridController::OnSelectTower2() { if (TowerData.IsValidIndex(1)) SetSelectedTower(TowerData[1]); }
void AGridController::OnSelectTower3() { if (TowerData.IsValidIndex(2)) SetSelectedTower(TowerData[2]); }
void AGridController::OnSelectTower4() { if (TowerData.IsValidIndex(3)) SetSelectedTower(TowerData[3]); }

bool AGridController::GetGridLocationUnderCursor(int32& OutX, int32& OutY)
{
	if (!GridManager) return false;

	FHitResult CursorHit;
	if (GetHitResultUnderCursor(ECC_Visibility, true, CursorHit))
	{
		FVector RelativeLocation = CursorHit.ImpactPoint - GridManager->GetActorLocation();
       
		float TileSize = GridManager->GetTileSize();
		float HalfTile = TileSize * 0.5f;

		OutX = FMath::FloorToInt((RelativeLocation.X + HalfTile) / TileSize);
		OutY = FMath::FloorToInt((RelativeLocation.Y + HalfTile) / TileSize);

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