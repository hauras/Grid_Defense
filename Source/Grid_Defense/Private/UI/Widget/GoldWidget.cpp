

#include "UI/Widget/GoldWidget.h"

#include "GameMode/GridGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "Components/TextBlock.h"

void UGoldWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AGridGameMode* GM = Cast<AGridGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
	if (GM)
	{

		UpdateGold(GM->GetCurrentGold());

		GM->OnGoldChanged.AddDynamic(this, &UGoldWidget::UpdateGold);

	}
}


void UGoldWidget::UpdateGold(int32 NewGold)
{
	if (GoldText) // 블루프린트에서 이름이 "GoldText"인 놈을 찾아서
	{
		GoldText->SetText(FText::AsNumber(NewGold)); // 글자만 숫자로 바꿔줌!
	}
}

