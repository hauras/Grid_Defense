

#include "UI/Widget/TowerToolTipWidget.h"
#include "Components/TextBlock.h"
#include "Data/TowerData.h"

void UTowerToolTipWidget::InitToolTip(UTowerData* TowerData)
{
	if (!TowerData) return;

	if (Txt_TowerName)
	{
		Txt_TowerName->SetText(FText::FromName(TowerData->TowerName));
	}

	if (Txt_TowerDamage)
	{
		int32 RoundedDamage = FMath::RoundToInt(TowerData->Damage);
		FString DamageStr = FString::Printf(TEXT("공격력 : %d"), RoundedDamage);
		Txt_TowerDamage->SetText(FText::FromString(DamageStr));
	}

	if (Txt_TowerCost)
	{
		FString CostStr = FString::Printf(TEXT("가격 : %d G"), TowerData->BuildCost);
		Txt_TowerCost->SetText(FText::FromString(CostStr));
	}
}
