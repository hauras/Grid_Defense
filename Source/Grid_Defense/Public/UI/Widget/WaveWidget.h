
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "WaveWidget.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UWaveWidget : public UUserWidget
{
	GENERATED_BODY()
protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Wave;

	UFUNCTION()
	void UpdateWaveText(int32 CurrentWave);
};
