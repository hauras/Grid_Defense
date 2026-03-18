
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" // 필수!
#include "EnemyBase.generated.h"

USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 드래곤 종류별로 다르게 줄 스탯들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHP = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MoveSpeed = 300.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	USkeletalMesh* EnemyMesh;
    
	// 필요하다면 공격력이나 보상 골드도 추가하세요!
};

UCLASS()
class GRID_DEFENSE_API AEnemyBase : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyBase();

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void Die();
	void InitializeStats();
	void MoveToTarget(FVector TargetLocation);
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* EnemyDataTable;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FName EnemyDataRowName;

	UFUNCTION(BlueprintCallable, Category = "Status")
	bool IsDead() const { return bIsDead; }
	
protected:
	virtual void BeginPlay() override;
	
	float CurrentHP;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TObjectPtr<UAnimMontage> DeathMontage;

	bool bIsDead = false;
};
