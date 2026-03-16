
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
protected:
	virtual void BeginPlay() override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	
	void OnMouseClick();

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<class UInputMappingContext> DefaultMappingContext;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ClickAction;

private:
	UPROPERTY()
	TObjectPtr<ATowerBase> CurrentPreviewActor;

	bool bBuildModeActive = true;
	
	UPROPERTY()
	TObjectPtr<AGridManager> GridManager;

	bool GetGridLocationUnderCursor(int32& OutX, int32& OutY);

	UPROPERTY(EditAnywhere, Category = "Tower")
	TObjectPtr<UTowerData> TowerData;

	void UpdateGhostVisual();
};
