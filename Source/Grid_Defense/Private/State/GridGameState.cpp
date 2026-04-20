

#include "State/GridGameState.h"
#include "UI/Widget/CardSelectWidget.h" // (경로는 본인 프로젝트에 맞게 수정!)
#include "Kismet/GameplayStatics.h"


void AGridGameState::ApplyCardBuff(FCardData PickedCard)
{
	AppliedBuff.Add(PickedCard);
	OnBuffUpdated.Broadcast(PickedCard);
}

TArray<FCardData> AGridGameState::GetRandomCards(int32 Count)
{
	TArray<FCardData> ResultCards;
    
	// 데이터 테이블이 세팅 안 되어 있으면 빈 배열 반환 (안전 처리)
	if (!CardDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("CardDataTable이 GameState에 세팅되지 않았습니다!"));
		return ResultCards;
	}

	// 1. 테이블의 모든 행 이름(Row Name)을 가져옵니다.
	TArray<FName> RowNames = CardDataTable->GetRowNames();
    
	if (RowNames.Num() == 0) return ResultCards;

	// 2. 피셔-예이츠(Fisher-Yates) 셔플 알고리즘으로 행 이름들을 섞어줍니다! (면접 어필 포인트 🌟)
	for (int32 i = RowNames.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		RowNames.Swap(i, j);
	}

	// 3. 섞인 리스트에서 맨 앞부터 Count(기본값 2) 개수만큼만 뽑아냅니다.
	int32 PickCount = FMath::Min(Count, RowNames.Num());
	for (int32 i = 0; i < PickCount; i++)
	{
		// 이름으로 실제 데이터(구조체)를 찾아옵니다.
		FCardData* RowData = CardDataTable->FindRow<FCardData>(RowNames[i], TEXT("RandomCardPick"));
		if (RowData)
		{
			ResultCards.Add(*RowData); // 배열에 쏙!
		}
	}

	return ResultCards;
}

void AGridGameState::ShowCardSelectUI()
{
	if (CardSelectWidgetClass)
	{
		// 1. 위젯 생성
		UCardSelectWidget* CardWidget = CreateWidget<UCardSelectWidget>(GetWorld(), CardSelectWidgetClass);
		if (CardWidget)
		{
			// 2. 화면에 띄우기
			CardWidget->AddToViewport();

			// 3. 마우스 커서 켜기 및 UI 전용 조작 모드 설정
			if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
			{
				PC->bShowMouseCursor = true;
                
				FInputModeUIOnly InputMode;
				InputMode.SetWidgetToFocus(CardWidget->TakeWidget());
				PC->SetInputMode(InputMode);
			}

			// 4. 게임 일시정지 (시간 멈춤)
			UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("CardSelectWidgetClass가 GameState에 세팅되지 않았습니다!"));
	}
}
