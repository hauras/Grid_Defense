
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameResultWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UGameResultWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Restart;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Quit;

	// 버튼을 눌렀을 때 실행될 함수들
	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnQuitClicked();	
};
