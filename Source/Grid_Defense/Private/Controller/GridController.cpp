#include "Controller/GridController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"
#include "Data/TowerData.h"
#include "Tower/TowerBase.h"
#include "Blueprint/UserWidget.h"

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
	InputModeData.SetHideCursorDuringCapture(false); // 드래그할 때 커서 숨김 방지
	SetInputMode(InputModeData);

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

		if (PauseAction)
		{
			EnhancedInputComponent->BindAction(PauseAction, ETriggerEvent::Triggered, this, &AGridController::TogglePause);
		}
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
	if (!SelectedTowerData || !bBuildModeActive) return;

	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
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

// 일시정지
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
		FVector RelativeLocation = CursorHit.ImpactPoint - GridManager->GetActorLocation();
       
		float TileSize = GridManager->GetTileSize(); // 이 Getter를 GridManager에 추가해야겠네요!
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