// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameSpeedWidget.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UGameSpeedWidget : public UUserWidget
{
	GENERATED_BODY()
protected:

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_Speed;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_Speed;

	int32 CurrentSpeedState = 1;

	UFUNCTION()
	void OnSpeedButtonClicked();
	
};
