
#include "Controller/GridController.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h" //  이거 꼭 추가되어 있어야 합니다!
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"

#include "DrawDebugHelpers.h" //  맨 위에 반드시 추가!
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
	bShowMouseCursor = true; // 커서는 계속 보이게 유지

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
	if (!GridManager) return;
	
	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
	{
		float TileSize = GridManager->TileSize;
		//FVector GridCenter = GridManager->GetActorLocation() + FVector(GridX * TileSize + TileSize * 0.5f, GridY * TileSize + TileSize * 0.5f, 0.0f);
		FVector GridCenter = GridManager->GetActorLocation() + FVector(GridX * TileSize, GridY * TileSize, 0.0f);
		if (bBuildModeActive && PreviewTower)
		{
			if (!CurrentPreviewActor)
			{
				CurrentPreviewActor = GetWorld()->SpawnActor<AActor>(PreviewTower, GridCenter, FRotator::ZeroRotator);				
			}
			else
			{
				CurrentPreviewActor->SetActorLocation(GridCenter);
			}
		}
	}
}

void AGridController::OnMouseClick()
{
	
	int32 GridX, GridY;
	if (GetGridLocationUnderCursor(GridX, GridY))
	{
		GridManager->AddTower(GridX, GridY);
	}
}

bool AGridController::GetGridLocationUnderCursor(int32& OutX, int32& OutY)
{
	if (!GridManager) return false;

	FHitResult CursorHit;
	// 다시 Visibility로 테스트합니다.
	if (GetHitResultUnderCursor(ECC_Visibility, true, CursorHit))
	{
		DrawDebugSphere(GetWorld(), CursorHit.ImpactPoint, 20.f, 16, FColor::Red, false, -1.f);

		FVector RelativeLocation = CursorHit.ImpactPoint - GridManager->GetActorLocation();
		OutX = FMath::RoundToInt(RelativeLocation.X / GridManager->TileSize);
        OutY = FMath::RoundToInt(RelativeLocation.Y / GridManager->TileSize);

		bool bIsValid = (OutX >= 0 && OutX < GridManager->GridWidth && OutY >= 0 && OutY < GridManager->GridHeight);
		
		return bIsValid;
	}

	return false;
}
