
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainMenuWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UMainMenuWidget : public UUserWidget
{
	GENERATED_BODY()
protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Start;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Load;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_QuitGame;

private:

	UFUNCTION()
	void OnStartClicked();

	UFUNCTION()
	void OnLoadClicked();

	UFUNCTION()
	void OnQuitClicked();
	
};
