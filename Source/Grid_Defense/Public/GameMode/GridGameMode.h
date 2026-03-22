
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GridGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChangedDelegate, int32, NewGold);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLifeChangedDelegate, int32, CurrentLife, int32, MaxLife);

/**
 * 
 */
UCLASS()
class GRID_DEFENSE_API AGridGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AGridGameMode();

	// 골드 관련 함수
	UFUNCTION(BlueprintCallable, Category = "Economy")
	void AddGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	bool SpendGold(int32 Amount);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	void DecreaseLife(int32 Damage);

	UFUNCTION(BlueprintCallable, Category = "Economy")
	int32 GetCurrentGold() const { return CurrentGold; }
	
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnGoldChangedDelegate OnGoldChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnLifeChangedDelegate OnLifeChanged;

	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	int32 CurrentLife;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	int32 MaxLife;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Status")
	int32 CurrentGold;


};
