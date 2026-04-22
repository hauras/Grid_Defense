
#include "UI/Widget/GameSpeedWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "GameMode/GridGameMode.h"
#include "Kismet/GameplayStatics.h"

void UGameSpeedWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Speed)
	{
		Btn_Speed->OnClicked.AddDynamic(this, &UGameSpeedWidget::OnSpeedButtonClicked);
	}
}

void UGameSpeedWidget::OnSpeedButtonClicked()
{
	if (AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld())))
	{
		// 2. "게임모드야, 속도 다음 단계로 넘겨줘!" 라고 명령하고 결과를 받습니다.
		float NewSpeed = GM->CycleGameSpeed();

		// 3. 받은 결과에 따라 화면 글자만 예쁘게 바꿔줍니다.
		if (Txt_Speed)
		{
			if (NewSpeed == 1.0f) Txt_Speed->SetText(FText::FromString(TEXT("▶ 1X")));
			else if (NewSpeed == 2.0f) Txt_Speed->SetText(FText::FromString(TEXT("▶▶ 2X")));
			else if (NewSpeed == 4.0f) Txt_Speed->SetText(FText::FromString(TEXT("▶▶▶ 4X")));
		}
	}
	
}
