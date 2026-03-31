
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildWidget.generated.h"

class UTowerData;
class UTowerSlotWidget;
class UHorizontalBox;

UCLASS()
class GRID_DEFENSE_API UBuildWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UHorizontalBox> TowerBox;

	UPROPERTY(EditAnywhere, Category = "Build UI")
	TSubclassOf<UTowerSlotWidget> SlotWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Build UI")
	TArray<UTowerData*> AvailableTowers;

protected:

	virtual void NativeConstruct() override;
};
