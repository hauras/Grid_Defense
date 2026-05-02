#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/GameStateBase.h"
#include "Engine/DataTable.h" 
#include "GridGameState.generated.h"

class UCardSelectWidget;

USTRUCT(BlueprintType)
struct FCardData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FString CardName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FString CardDescription;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
    FGameplayTag TowerTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
    float DamageBuffAmount = 0.0f;

    // [수정] AttackRangeBuffAmount 제거

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
    float AttackSpeedBuffAmount = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
    int32 ChainCountBuffAmount = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
    float SplashRadiusBuffAmount = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuffUpdated, const FCardData&, CardInfo); 
 
UCLASS()
class GRID_DEFENSE_API AGridGameState : public AGameStateBase
{
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintAssignable, Category = "Buff")
    FOnBuffUpdated OnBuffUpdated;

    // [수정] 값 전달 → const 참조 전달
    UFUNCTION(BlueprintCallable, Category = "Buff")
    void ApplyCardBuff(const FCardData& PickedCard);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
    TObjectPtr<UDataTable> CardDataTable;

    UFUNCTION(BlueprintCallable, Category = "Card")
    TArray<FCardData> GetRandomCards(int32 Count = 2);

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UCardSelectWidget> CardSelectWidgetClass;

    UFUNCTION(BlueprintCallable, Category = "UI")
    void ShowCardSelectUI();

    UPROPERTY(BlueprintReadOnly, Category = "Card")
    TArray<FCardData> AppliedBuff;

private:
    // [수정] 위젯 캐싱
    UPROPERTY()
    TObjectPtr<UCardSelectWidget> CachedCardSelectWidget;
};
