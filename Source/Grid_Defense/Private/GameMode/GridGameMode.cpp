
#include "GameMode/GridGameMode.h"

AGridGameMode::AGridGameMode()
{
	// 추후 데이터 테이블로 변경
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
	// 1번 경로: 돈이 충분할 때
	if (Amount > 0 && CurrentGold >= Amount)
	{
		CurrentGold -= Amount;
		OnGoldChanged.Broadcast(CurrentGold);
		UE_LOG(LogTemp, Log, TEXT("골드 소모! 남은 골드: %d"), CurrentGold);
        
		return true; // 💡 여기서 함수가 종료됨
	}
    
	// 2번 경로: 돈이 부족하거나 잘못된 값이 들어왔을 때 (if문을 통과해버린 경우)
	UE_LOG(LogTemp, Warning, TEXT("잔액 부족! 타워 건설 실패."));
    
	// 🚨 핵심! 이 녀석이 무조건 if문 괄호(}) 바깥, 즉 함수의 맨 마지막에 있어야 합니다!
	return false; 
}

void AGridGameMode::DecreaseLife(int32 Damage)
{
	if (CurrentLife > 0 && Damage > 0)
	{
		CurrentLife -= Damage;
		OnLifeChanged.Broadcast(CurrentLife, MaxLife);
		UE_LOG(LogTemp, Warning, TEXT("본진 피격! 남은 체력: %d / %d"), CurrentLife, MaxLife);

		if (CurrentLife <= 0)
		{
			CurrentLife = 0;
			UE_LOG(LogTemp, Error, TEXT(" 게임 오버! 본진 파괴됨 💥"));
			// TODO: 나중에 여기에 게임 오버 UI 띄우기 및 일시정지 로직 추가
		}
	}
}

void AGridGameMode::SetCurrentGold(int32 NewGold)
{
	CurrentGold = NewGold;
	// 🌟 덮어씌운 다음, UI 위젯들이 숫자를 새로고침하도록 방송합니다!
	OnGoldChanged.Broadcast(CurrentGold);
}

void AGridGameMode::SetCurrentLife(int32 NewLife)
{
	CurrentLife = NewLife;
	// 🌟 생명력 UI도 새로고침!
	OnLifeChanged.Broadcast(CurrentLife, MaxLife);
}

