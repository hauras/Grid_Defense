

#include "UI/Widget/BuildWidget.h"
#include "Components/HorizontalBox.h"
#include "UI/Widget/TowerSlotWidget.h"

void UBuildWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (!TowerBox || !SlotWidgetClass) return;

	// 혹시 모르니 상자 안을 한 번 깨끗하게 비움
	TowerBox->ClearChildren(); 

	// 데이터 배열을 돌면서 슬롯을 하나씩 생성!
	for (UTowerData* Data : AvailableTowers)
	{
		if (Data)
		{
			// 1. 슬롯 위젯 생성
			UTowerSlotWidget* NewSlot = CreateWidget<UTowerSlotWidget>(this, SlotWidgetClass);
          
			if (NewSlot)
			{
				// 2. 데이터 주입
				NewSlot->InitSlot(Data); 
             
				// 3. 가로 정렬 상자에 쏙 넣기
				TowerBox->AddChildToHorizontalBox(NewSlot); 
			}
		}
	}
}
