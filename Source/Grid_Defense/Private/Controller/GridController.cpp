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
		float TileSize = GridManager->TileSize;
		
		FVector GridCenter = GridManager->GetActorLocation() + 
							 FVector(GridX * TileSize, GridY * TileSize, 50.0f);

		if (!CurrentPreviewActor)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
			CurrentPreviewActor = GetWorld()->SpawnActor<ATowerBase>(TowerData->TowerActorClass, GridCenter, FRotator::ZeroRotator, SpawnParams);
			
			if (CurrentPreviewActor)
			{
				CurrentPreviewActor->InitTower(TowerData, true);
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
		FVector RelativeLocation = CursorHit.ImpactPoint - GridManager->GetActorLocation();
		
		float TileSize = GridManager->TileSize;
		float HalfTile = TileSize * 0.5f;

		OutX = FMath::FloorToInt((RelativeLocation.X + HalfTile) / TileSize);
		OutY = FMath::FloorToInt((RelativeLocation.Y + HalfTile) / TileSize);

		return (OutX >= 0 && OutX < GridManager->GridWidth && OutY >= 0 && OutY < GridManager->GridHeight);
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