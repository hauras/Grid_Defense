
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "State/GridGameState.h"
#include "CardSelectWidget.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UCardSelectWidget : public UUserWidget
{
	GENERATED_BODY()
protected:
	virtual void NativeConstruct() override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_LeftCard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Btn_RightCard;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeftName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RightName;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_LeftDesc;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> Text_RightDesc;

	UFUNCTION()
	void OnLeftCardClicked();

	UFUNCTION()
	void OnRightCardClicked();

private:
	// 뽑힌 카드를 저장해둘 배열
	TArray<FCardData> PickedCards;	
	
};
