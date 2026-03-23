
#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "GridAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UGridAssetManager : public UAssetManager
{
	GENERATED_BODY()
public:

	static UGridAssetManager& Get();

protected:

	virtual void StartInitialLoading() override;	
};
