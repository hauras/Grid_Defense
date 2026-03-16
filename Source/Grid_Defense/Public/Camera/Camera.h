
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h" 
#include "Camera.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class GRID_DEFENSE_API ACamera : public APawn
{
	GENERATED_BODY()

public:
	ACamera();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Move(const FInputActionValue& Value);
	void Zoom(const FInputActionValue& Value);
	
protected:

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USceneComponent> RootScene;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, Category = "Camera")
	TObjectPtr<UCameraComponent> Camera;
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ZoomAction;

	// 카메라 설정
	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float ZoomSpeed = 100.f;

	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float MinZoom = 500.f;

	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float MaxZoom = 3000.f;

	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float MoveSpeed = 1500.f;
	
	float TargetZoom;

	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	bool bEnableScroll = true;

	UPROPERTY(EditAnywhere, Category = "Camera Settings")
	float Margin = 50.f;

	void HandleScroll(float DeltaTime);
};
