

#include "UI/Widget/EnemyHPWidget.h"
#include "Components/ProgressBar.h" // 💡 프로그레스 바 헤더 필수 포함!
#include "Enemy/EnemyBase.h"

void UEnemyHPWidget::UpdateHP(float CurrentHP, float MaxHP)
{
	if (HPProgressBar && MaxHP > 0.f)
	{
		float HPRatio = CurrentHP / MaxHP;
		HPProgressBar->SetPercent(HPRatio);
	}
}

void UEnemyHPWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
}

void UEnemyHPWidget::OnHPChanged(float CurrentHP, float MaxHP)
{
	UpdateHP(CurrentHP, MaxHP);
}
