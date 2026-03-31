

#include "UI/Widget/TowerSlotWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Controller/GridController.h"
#include "Data/TowerData.h"
#include "UI/Widget/TowerToolTipWidget.h"

void UTowerSlotWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (Button_TowerSlot)
	{
		Button_TowerSlot->OnClicked.AddDynamic(this, &UTowerSlotWidget::OnSlotClicked);
	}
}

void UTowerSlotWidget::InitSlot(UTowerData* InData)
{
	MyData = InData;
    
	if (MyData && Img_Icon && MyData->TowerIcon)
	{
		Img_Icon->SetBrushFromTexture(MyData->TowerIcon);
	}
	if (TooltipClass)
	{
		// 1. 툴팁 위젯을 메모리에 생성합니다.
		UTowerToolTipWidget* TooltipWidget = CreateWidget<UTowerToolTipWidget>(this, TooltipClass);
        
		if (TooltipWidget)
		{
			// 2. 툴팁에게 "내 데이터 좀 그려줘!" 하고 데이터를 넘깁니다.
			TooltipWidget->InitToolTip(MyData);

			// 3. 버튼에 툴팁으로 찰싹 붙입니다! (마우스 올리면 언리얼이 알아서 띄워줌)
			Button_TowerSlot->SetToolTip(TooltipWidget);
		}
	}
}


void UTowerSlotWidget::OnSlotClicked()
{
	if (MyData)
	{
		if (AGridController* PC = Cast<AGridController>(GetOwningPlayer()))
		{
			PC->SetSelectedTower(MyData);
		}
	}
}
