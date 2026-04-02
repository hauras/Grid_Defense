
#include "UI/DamageText/DamageTextComponent.h"
#include "UI/DamageText/DamageTextWidget.h"

void UDamageTextComponent::SetDamageText(float Damage, FLinearColor TextColor)
{
	if (UDamageTextWidget* MyWidget = Cast<UDamageTextWidget>(GetUserWidgetObject()))
	{
		MyWidget->UpdateText(Damage, TextColor);
	}
}

void UDamageTextComponent::HideDamageText()
{
	if (UDamageTextWidget* MyWidget = Cast<UDamageTextWidget>(GetUserWidgetObject()))
	{
		MyWidget->HideText();
	}
}
