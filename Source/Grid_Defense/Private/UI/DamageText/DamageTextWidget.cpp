

#include "UI/DamageText/DamageTextWidget.h"

#include "Components/TextBlock.h"

void UDamageTextWidget::UpdateText(float Damage, FLinearColor Color)
{
	if (DamageText)
	{
		FString DamageStr = FString::Printf(TEXT("%.0f"), Damage);
		DamageText->SetText(FText::FromString(DamageStr));

		DamageText->SetColorAndOpacity(FSlateColor(Color));
		DamageText->SetRenderOpacity(1.0f);
		DamageText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
}

void UDamageTextWidget::HideText()
{
	if (DamageText)
	{
		DamageText->SetVisibility(ESlateVisibility::Hidden);
	}
}
