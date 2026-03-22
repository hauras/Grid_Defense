
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "GridHUD.generated.h"

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API AGridHUD : public AHUD
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "UI")
	TSubclassOf<class UUserWidget> MainHUDClass;

protected:
	virtual void BeginPlay() override;

private:
	// 생성된 위젯 인스턴스 보관
	UPROPERTY()
	class UUserWidget* MainHUDWidget;	
};
