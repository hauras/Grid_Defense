

#include "GridAssetManager.h"
#include "GridGameplayTags.h"

UGridAssetManager& UGridAssetManager::Get()
{
	check(GEngine);
	
	UGridAssetManager* GridAssetManager = Cast<UGridAssetManager>(GEngine->AssetManager);
	return *GridAssetManager;
}

void UGridAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();

	FGridGameplayTags::InitializeNativeGameplayTags();
}
