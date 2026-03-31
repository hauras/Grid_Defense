
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GridController.generated.h"

class ATowerBase;
class UTowerData;
class UInputAction;
class AGridManager;
/**
 * 
 */
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
	
	void OnKey1Pressed();
	void OnKey2Pressed();
	void OnKey3Pressed();
	void OnKey4Pressed();

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
	UTowerData* SelectedTowerData;
	
	void UpdateGhostVisual();
};
