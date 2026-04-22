
#include "GameMode/GridGameMode.h"
#include "Blueprint/UserWidget.h"    // 위젯 생성용
#include "Kismet/GameplayStatics.h"

AGridGameMode::AGridGameMode()
{
	CurrentGold = 100;
	MaxLife = 50;
	CurrentLife = MaxLife;
}

void AGridGameMode::BeginPlay()
{
	Super::BeginPlay();

	OnGoldChanged.Broadcast(CurrentGold);
	OnLifeChanged.Broadcast(CurrentLife, MaxLife);
}


void AGridGameMode::AddGold(int32 Amount)
{
	if (Amount > 0)
	{
		CurrentGold += Amount;
		OnGoldChanged.Broadcast(CurrentGold);
		UE_LOG(LogTemp, Log, TEXT("골드 획득! 현재 골드: %d"), CurrentGold);		
	}
}

bool AGridGameMode::SpendGold(int32 Amount)
{
	if (Amount > 0 && CurrentGold >= Amount)
	{
		CurrentGold -= Amount;
		OnGoldChanged.Broadcast(CurrentGold);
		UE_LOG(LogTemp, Log, TEXT("골드 소모! 남은 골드: %d"), CurrentGold);
        
		return true; // 💡 여기서 함수가 종료됨
	}
    
	UE_LOG(LogTemp, Warning, TEXT("잔액 부족! 타워 건설 실패."));
    
	return false; 
}

void AGridGameMode::DecreaseLife(int32 Damage)
{
	if (CurrentLife > 0 && Damage > 0)
	{
		CurrentLife -= Damage;

		if (CurrentLife <= 0)
		{
			CurrentLife = 0;
			GameOver();
		}

		OnLifeChanged.Broadcast(CurrentLife, MaxLife);
		UE_LOG(LogTemp, Warning, TEXT("본진 피격! 남은 체력: %d / %d"), CurrentLife, MaxLife);
	}
}

void AGridGameMode::SetCurrentGold(int32 NewGold)
{
	CurrentGold = NewGold;
	OnGoldChanged.Broadcast(CurrentGold);
}

void AGridGameMode::SetCurrentLife(int32 NewLife)
{
	CurrentLife = NewLife;
	OnLifeChanged.Broadcast(CurrentLife, MaxLife);
}


void AGridGameMode::GameOver()
{
	UE_LOG(LogTemp, Error, TEXT("☠️ 게임 오버! 본진 파괴됨!"));
    
	// 1. 게임 시간 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);
    
	// 2. C++에서 직접 위젯 생성 및 화면에 추가
	if (GameOverUIClass)
	{
		UUserWidget* GameOverWidget = CreateWidget<UUserWidget>(GetWorld(), GameOverUIClass);
		if (GameOverWidget)
		{
			GameOverWidget->AddToViewport();
            
			// 3. 마우스 커서 켜고, 게임 조작 막기 (UI 전용 모드)
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				PC->SetShowMouseCursor(true);
                
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(GameOverWidget->TakeWidget());
				PC->SetInputMode(InputMode);
			}
		}
	}
}

void AGridGameMode::GameClear()
{
	UE_LOG(LogTemp, Warning, TEXT("🎉 모든 웨이브 클리어! 게임 승리!"));
    
	// 1. 게임 시간 정지
	UGameplayStatics::SetGamePaused(GetWorld(), true);
    
	// 2. C++에서 직접 위젯 생성 및 화면에 추가
	if (GameClearUIClass)
	{
		UUserWidget* GameClearWidget = CreateWidget<UUserWidget>(GetWorld(), GameClearUIClass);
		if (GameClearWidget)
		{
			GameClearWidget->AddToViewport();
            
			// 3. 마우스 커서 켜고, 게임 조작 막기 (UI 전용 모드)
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				PC->SetShowMouseCursor(true);
                
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(GameClearWidget->TakeWidget());
				PC->SetInputMode(InputMode);
			}
		}
	}
}

float AGridGameMode::CycleGameSpeed()
{
	// 1 -> 2 -> 4 -> 1 순환
	if (CurrentSpeedState == 1) CurrentSpeedState = 2;
	else if (CurrentSpeedState == 2) CurrentSpeedState = 4;
	else CurrentSpeedState = 1;

	// 배속 적용
	float TimeScale = CurrentSpeedState;
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), TimeScale);
    
	UE_LOG(LogTemp, Warning, TEXT("게임 배속 변경: %fx"), TimeScale);

	// 바뀐 속도를 UI한테 알려줍니다!
	return TimeScale; 
}
