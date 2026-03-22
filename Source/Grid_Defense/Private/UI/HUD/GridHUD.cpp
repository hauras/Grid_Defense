

#include "UI/HUD/GridHUD.h"
#include "Blueprint/UserWidget.h"

void AGridHUD::BeginPlay()
{
	Super::BeginPlay();

	if (MainHUDClass)
	{
		// 1. 위젯 생성
		MainHUDWidget = CreateWidget<UUserWidget>(GetWorld(), MainHUDClass);
		
		if (MainHUDWidget)
		{
			// 2. 화면에 표시
			MainHUDWidget->AddToViewport();
		}
	}
}
