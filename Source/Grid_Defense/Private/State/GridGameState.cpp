#include "State/GridGameState.h"
#include "UI/Widget/CardSelectWidget.h"
#include "Kismet/GameplayStatics.h"

void AGridGameState::ApplyCardBuff(const FCardData& PickedCard)
{
	AppliedBuff.Add(PickedCard);
	OnBuffUpdated.Broadcast(PickedCard);
}

TArray<FCardData> AGridGameState::GetRandomCards(int32 Count)
{
	TArray<FCardData> ResultCards;

	if (!CardDataTable)
	{
		return ResultCards;
	}

	TArray<FName> RowNames = CardDataTable->GetRowNames();
	if (RowNames.Num() == 0) return ResultCards;

	for (int32 i = RowNames.Num() - 1; i > 0; i--)
	{
		int32 j = FMath::RandRange(0, i);
		RowNames.Swap(i, j);
	}

	int32 PickCount = FMath::Min(Count, RowNames.Num());
	for (int32 i = 0; i < PickCount; i++)
	{
		FCardData* RowData = CardDataTable->FindRow<FCardData>(RowNames[i], TEXT("RandomCardPick"));
		if (RowData)
		{
			ResultCards.Add(*RowData);
		}
	}

	return ResultCards;
}

void AGridGameState::ShowCardSelectUI()
{
	if (!CardSelectWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("CardSelectWidgetClass가 GameState에 세팅되지 않았습니다!"));
		return;
	}

	if (!CachedCardSelectWidget)
	{
		CachedCardSelectWidget = CreateWidget<UCardSelectWidget>(GetWorld(), CardSelectWidgetClass);
	}

	if (CachedCardSelectWidget)
	{
		CachedCardSelectWidget->AddToViewport();

		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PC->bShowMouseCursor = true;

			FInputModeUIOnly InputMode;
			InputMode.SetWidgetToFocus(CachedCardSelectWidget->TakeWidget());
			PC->SetInputMode(InputMode);
		}

		UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 0.0f);
	}
}