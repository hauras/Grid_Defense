
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerSellWidget.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UTowerSellWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
    void SetTowerInfo(int32 InGridX, int32 InGridY, int32 InRefundAmount);

protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Sell;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TXT_Sell;
	
private:
	UFUNCTION()
	void OnSellClicked();

	int32 GridX = 0;
	int32 GridY = 0;
	int32 RefundAmount = 0;
};
