

#include "UI/Widget/CardSelectWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"

void UCardSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_LeftCard)
	{
		Btn_LeftCard->OnClicked.AddDynamic(this, &UCardSelectWidget::OnLeftCardClicked);
	}
	if (Btn_RightCard)
	{
		Btn_RightCard->OnClicked.AddDynamic(this, &UCardSelectWidget::OnRightCardClicked);
	}

	// 2. 게임스테이트에서 카드 뽑아오기
	if (AGridGameState* GS = Cast<AGridGameState>(UGameplayStatics::GetGameState(this)))
	{
		PickedCards = GS->GetRandomCards(2);

		// 3. 텍스트 컴포넌트에 글씨 세팅
		if (PickedCards.Num() >= 2)
		{
			if (Text_LeftName) Text_LeftName->SetText(FText::FromString(PickedCards[0].CardName));
			if (Text_LeftDesc) Text_LeftDesc->SetText(FText::FromString(PickedCards[0].CardDescription));
            
			if (Text_RightName) Text_RightName->SetText(FText::FromString(PickedCards[1].CardName));
			if (Text_RightDesc) Text_RightDesc->SetText(FText::FromString(PickedCards[1].CardDescription));
		}
	}
}

void UCardSelectWidget::OnLeftCardClicked()
{
	if (PickedCards.Num() > 0)
	{
		// 2. 방송국(GameState)을 찾아서 0번(왼쪽) 카드 데이터를 넘겨줍니다!
		if (AGridGameState* GS = Cast<AGridGameState>(UGameplayStatics::GetGameState(this)))
		{
			GS->ApplyCardBuff(PickedCards[0]); 
		}
	}
    
	// 3. 멈춰있던 게임 시간을 다시 흐르게 합니다. (1.0 = 정상 속도)
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
    
	// (선택) 마우스 커서를 끄고 게임 조작 모드로 돌려줍니다.
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeGameAndUI InputMode;
        
		// (선택) 마우스 클릭 시 포커스를 잃지 않도록 설정
		InputMode.SetHideCursorDuringCapture(false); 
        
		PC->SetInputMode(InputMode);
        
		// 2. 마우스 커서를 끄지 말고 'True'로 켜둡니다!
		PC->bShowMouseCursor = true;
	}

	// 4. 화면에서 이 위젯(카드 선택 창)을 지워버립니다.
	RemoveFromParent();
}

void UCardSelectWidget::OnRightCardClicked()
{
	if (PickedCards.Num() > 1)
	{
		if (AGridGameState* GS = Cast<AGridGameState>(UGameplayStatics::GetGameState(this)))
		{
			GS->ApplyCardBuff(PickedCards[1]); // 1번(오른쪽) 카드 데이터 전달!
		}
	}
    
	UGameplayStatics::SetGlobalTimeDilation(this, 1.0f);
    
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0))
	{
		FInputModeGameAndUI InputMode;
        
		// (선택) 마우스 클릭 시 포커스를 잃지 않도록 설정
		InputMode.SetHideCursorDuringCapture(false); 
        
		PC->SetInputMode(InputMode);
        
		// 2. 마우스 커서를 끄지 말고 'True'로 켜둡니다!
		PC->bShowMouseCursor = true;
	}

	RemoveFromParent();
}
