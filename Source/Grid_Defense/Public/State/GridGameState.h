
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

	// 대상 타워를 지정하는 태그
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card")
	FGameplayTag TowerTag;

	// --- 여기서부터 버프 종류별 변수 추가 ---
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
	float DamageBuffAmount = 0.0f;    // 공격력 증가량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
	float AttackRangeBuffAmount = 0.0f; // 사거리 증가량

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
	float AttackSpeedBuffAmount = 0.0f; // 공격 속도 증가량 (Interval 감소량)

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
	int32 ChainCountBuffAmount = 0;   // ⚡ 번개 타워 전용: 전이 횟수 추가

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Card|Stats")
	float SplashRadiusBuffAmount = 0.0f; // 💥 스플래시 타워 전용: 폭발 범위 추가
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBuffUpdated, const FCardData&, CardInfo); 
 
UCLASS()
class GRID_DEFENSE_API AGridGameState : public AGameStateBase
{
	GENERATED_BODY()
public:

	// (참고: 이제 GameState가 데미지만 관리하지 않으니 DamageMultiplier 변수는 지우셔도 무방합니다!)

	UPROPERTY(BlueprintAssignable, Category = "Buff")
	FOnBuffUpdated OnBuffUpdated;

	// 💡 기존의 UpdateGlobalDamage 대신, 카드를 통째로 넘기는 함수로 변경!
	UFUNCTION(BlueprintCallable, Category = "Buff")
	void ApplyCardBuff(FCardData PickedCard); 

	// 카드 데이터 테이블
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Card")
	TObjectPtr<UDataTable> CardDataTable;

	// 무작위 카드 뽑기
	UFUNCTION(BlueprintCallable, Category = "Card")
	TArray<FCardData> GetRandomCards(int32 Count = 2);

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UCardSelectWidget> CardSelectWidgetClass;

	// 💡 웨이브가 끝날 때 호출할 '카드 창 띄우기' 함수
	UFUNCTION(BlueprintCallable, Category = "UI")
	void ShowCardSelectUI();

	UPROPERTY(BlueprintReadOnly, Category = "Card")
	TArray<FCardData> AppliedBuff;
};

