
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
	// 블루프린트의 BeginPlay와 같은 역할 (위젯이 화면에 뜰 때 실행됨)
	virtual void NativeConstruct() override;

	// 🌟 [핵심] UMG 디자이너에서 만든 버튼과 C++ 변수를 이름표로 연결해 줍니다!
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
