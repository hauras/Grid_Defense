

#include "UI/Widget/TowerSellWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Grid/GridManager.h"
#include "Kismet/GameplayStatics.h"

void UTowerSellWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Btn_Sell)
	{
		Btn_Sell->OnClicked.AddDynamic(this, &UTowerSellWidget::OnSellClicked);
	}
}

void UTowerSellWidget::SetTowerInfo(int32 InGridX, int32 InGridY, int32 InRefundAmount)
{
	GridX = InGridX;
	GridY = InGridY;
	RefundAmount = InRefundAmount;

	TXT_Sell->SetText(FText::FromString(FString::Printf(TEXT("환급 금액: %d"), RefundAmount)));

}

void UTowerSellWidget::OnSellClicked()
{
	AGridManager* GridManager = Cast<AGridManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AGridManager::StaticClass()));
	if (GridManager)
	{
		GridManager->RemoveTower(GridX, GridY);
	}

	// 입력 모드 복구
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
	}

	RemoveFromParent();
}
