#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHPWidget.generated.h"

class UProgressBar;
class UImage;

UCLASS()
class GRID_DEFENSE_API UEnemyHPWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	void UpdateHP(float CurrentHP, float MaxHP);

	// 슬로우 아이콘 표시/숨김
	UFUNCTION(BlueprintCallable)
	void SetSlowVisible(bool bVisible);

	UFUNCTION()
	void OnHPChanged(float CurrentHP, float MaxHP);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPProgressBar;

	// WBP_EnemyHP 위젯에 Image 컴포넌트 추가 후 이름을 SlowIcon으로 맞춰주세요
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> SlowIcon;

	virtual void NativeConstruct() override;
};