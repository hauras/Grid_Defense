

#include "UI/Widget/WaveWidget.h"
#include "Components/TextBlock.h"
#include "Enemy/EnemySpawner.h"
#include "Kismet/GameplayStatics.h"

void UWaveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	AActor* SpawnerActor = UGameplayStatics::GetActorOfClass(GetWorld(), AEnemySpawner::StaticClass());
	if (AEnemySpawner* Spawner = Cast<AEnemySpawner>(SpawnerActor))
	{
		Spawner->OnWaveChanged.AddDynamic(this, &UWaveWidget::UpdateWaveText);
	}
}

void UWaveWidget::UpdateWaveText(int32 CurrentWave)
{
	if (Txt_Wave)
	{
		FString FormattedString = FString::Printf(TEXT("WAVE %d"), CurrentWave);
		Txt_Wave->SetText(FText::FromString(FormattedString));
	}
}
