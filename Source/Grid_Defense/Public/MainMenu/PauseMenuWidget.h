
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuWidget.generated.h"

class UButton;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UPauseMenuWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Resume;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Save;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_ToMain;

private:
	UFUNCTION()
	void OnResumeClicked();

	UFUNCTION()
	void OnSaveClicked();

	UFUNCTION()
	void OnToMainClicked();	
};
