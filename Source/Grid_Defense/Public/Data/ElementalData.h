#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "ElementalData.generated.h"

USTRUCT(BlueprintType)
struct FElementalMatchup : public FTableRowBase
{
	GENERATED_BODY()

	// 공격 속성 태그 (예: Damage_Fire)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag AttackTag;

	// 피격 적 속성 태그 (예: Enemy_Nature)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FGameplayTag EnemyTag;

	// 데미지 배율 (1.5 = 150%, 0.5 = 50%)
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DamageMultiplier = 1.0f;

	// 데미지 텍스트 색상
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FLinearColor TextColor = FLinearColor::White;
};