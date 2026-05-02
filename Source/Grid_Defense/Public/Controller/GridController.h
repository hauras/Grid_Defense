#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GridController.generated.h"

class ATowerBase;
class UTowerData;
class UInputAction;
class AGridManager;
class UTowerSellWidget;

UCLASS()
class GRID_DEFENSE_API AGridController : public APlayerController
{
	GENERATED_BODY()

public:
	AGridController();

	void CursorTrace();

	UFUNCTION(BlueprintCallable, Category = "Build")
	void SetSelectedTower(UTowerData* NewData);

protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;

	void OnMouseClick();

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ClickAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> PauseAction;

	UPROPERTY(EditAnywhere, Category = "Input|Tower Select")
	TObjectPtr<UInputAction> SelectTower1Action;

	UPROPERTY(EditAnywhere, Category = "Input|Tower Select")
	TObjectPtr<UInputAction> SelectTower2Action;

	UPROPERTY(EditAnywhere, Category = "Input|Tower Select")
	TObjectPtr<UInputAction> SelectTower3Action;

	UPROPERTY(EditAnywhere, Category = "Input|Tower Select")
	TObjectPtr<UInputAction> SelectTower4Action;

	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UUserWidget> PauseMenuClass;

	// [추가] 타워 판매 위젯 클래스
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<UTowerSellWidget> TowerSellWidgetClass;

	void TogglePause();

	void OnSelectTower1();
	void OnSelectTower2();
	void OnSelectTower3();
	void OnSelectTower4();

private:
	UPROPERTY()
	TObjectPtr<ATowerBase> CurrentPreviewActor;

	bool bBuildModeActive = true;

	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;

	bool GetGridLocationUnderCursor(int32& OutX, int32& OutY);

	UPROPERTY(EditAnywhere, Category = "Tower")
	TArray<TObjectPtr<UTowerData>> TowerData;

	UPROPERTY()
	TObjectPtr<UTowerData> SelectedTowerData;

	UPROPERTY()
	TObjectPtr<UTowerSellWidget> ActiveSellWidget;
	
	void UpdateGhostVisual();
	void ShowSellWidget(int32 GridX, int32 GridY);

	UFUNCTION()
	void CloseSellWidget();
};