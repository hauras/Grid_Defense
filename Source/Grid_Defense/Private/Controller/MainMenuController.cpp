

#include "Controller/MainMenuController.h"

void AMainMenuController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;

	// 🌟 UI만 조작할 수 있도록 입력 모드 변경
	FInputModeUIOnly InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputModeData);
}
