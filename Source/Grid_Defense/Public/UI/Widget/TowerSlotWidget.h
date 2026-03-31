
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerSlotWidget.generated.h"

class UImage;
class UTowerData;
class UButton;
class UTowerToolTipWidget;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UTowerSlotWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button_TowerSlot;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> Img_Icon;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UTowerToolTipWidget> TooltipClass;
	
	void InitSlot(UTowerData* TowerData);

protected:
	virtual void NativeConstruct() override;

private:

	UPROPERTY()
	TObjectPtr<UTowerData> MyData;

	UFUNCTION()
	void OnSlotClicked();
};
