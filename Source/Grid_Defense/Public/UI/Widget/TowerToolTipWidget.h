// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TowerToolTipWidget.generated.h"

class UTextBlock;
class UTowerData;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UTowerToolTipWidget : public UUserWidget
{
	GENERATED_BODY()
public:

	void InitToolTip(UTowerData* TowerData);

protected:

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_TowerName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_TowerDamage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Txt_TowerCost;

	
};
