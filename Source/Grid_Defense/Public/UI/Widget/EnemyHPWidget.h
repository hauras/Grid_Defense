
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EnemyHPWidget.generated.h"

class UProgressBar;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UEnemyHPWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	void UpdateHP(float CurrentHP, float MaxHP);
	
	UFUNCTION()
	void OnHPChanged(float CurrentHP, float MaxHP);
	
protected:
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> HPProgressBar;

	virtual void NativeConstruct() override;

	
};
