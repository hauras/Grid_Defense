#include "UI/Widget/EnemyHPWidget.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Enemy/EnemyBase.h"

void UEnemyHPWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 시작 시 슬로우 아이콘 숨김
	if (SlowIcon)
	{
		SlowIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UEnemyHPWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPProgressBar && MaxHP > 0.f)
	{
		float HPRatio = CurrentHP / MaxHP;
		HPProgressBar->SetPercent(HPRatio);
	}
}

void UEnemyHPWidget::OnHPChanged(float CurrentHP, float MaxHP)
{
	UpdateHP(CurrentHP, MaxHP);
}

void UEnemyHPWidget::SetSlowVisible(bool bVisible)
{
	if (SlowIcon)
	{
		SlowIcon->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}