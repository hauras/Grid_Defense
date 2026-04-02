
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DamageTextWidget.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UDamageTextWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;

	void UpdateText(float Damage, FLinearColor Color);
	void HideText();
};
