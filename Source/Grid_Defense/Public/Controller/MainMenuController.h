
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainMenuController.generated.h"

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API AMainMenuController : public APlayerController
{
	GENERATED_BODY()
protected:


	virtual void BeginPlay() override;
};
