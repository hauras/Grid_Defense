
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GridSaveGame.generated.h"

enum class ETileType : uint8;
class UTowerData;

USTRUCT(BlueprintType)
struct FTowerSaveData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UTowerData> TowerData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridX;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 GridY;

	
};
/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API UGridSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UGridSaveGame();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<FTowerSaveData> SavedTowers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 UserIndex;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	TArray<ETileType> SavedMapLayout;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 SavedGold;

	// 웨이브 번호
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	int32 SavedWaveNumber;

	// 넥서스 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SaveData")
	float SavedNexusHP;
};
