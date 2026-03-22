
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GoldWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UGoldWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UFUNCTION()
	void UpdateGold(int32 NewGold);

protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> GoldText;
};
