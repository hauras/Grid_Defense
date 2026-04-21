#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" 
#include "GameplayTagContainer.h"    
#include "EnemyBase.generated.h"

class UDamageTextComponent;
class AGridManager;
class UWidgetComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChangedDelegate, float, CurrentHP, float, MaxHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnEnemyDiedDelegate);


USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Class")
    TSubclassOf<class AEnemyBase> EnemyClass;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MoveSpeed = 800.f;

    // 💡 추가 1: 돈 보상
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    int32 GoldReward = 10;

    // 💡 추가 2: 기지 공격력 (목숨 깎는 양)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    int32 LifeDamage = 1;

    // 💡 추가 3: 타겟팅 중요도 (기본값 1, 보스는 5)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float TargetingWeight = 1.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
    FGameplayTagContainer EnemyTags;
};

UCLASS()
class GRID_DEFENSE_API AEnemyBase : public ACharacter
{
    GENERATED_BODY()

public:
    AEnemyBase();

    virtual void Tick(float DeltaTime) override; 
    
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    void InitializeStats();
    void SetPath(const TArray<FVector>& NewPath); 
    void RecalculatePath();
    
    UFUNCTION(BlueprintCallable, Category = "Stat")
    int32 GetLifeDamage() const { return MyLifeDamage; }

    // 🌟 [추가 2] 넥서스 골인 전용 함수 (돈 안 줌, 애니메이션 안 틈)
    void ReachNexus();
    UFUNCTION(BlueprintCallable, Category = "Status")
    bool IsDead() const { return bIsDead; }

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHPChangedDelegate OnHPChanged;
    
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnEnemyDiedDelegate OnEnemyDied;
    
    void InitializeEnemy(FName InRowName);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
    FGameplayTagContainer GameplayTags;

    void ApplySlow(float SlowDuration);

    UFUNCTION(BlueprintCallable, Category = "Status")
    float GetCurrentHP() const { return CurrentHP; }

    UFUNCTION(BlueprintCallable, Category = "Status")
    float GetMaxHP() const { return MaxHP; }

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tag")
    FGameplayTagContainer EnemyTags;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI")
    TObjectPtr<UDamageTextComponent> DamageTextComp;

    float AccumulatedDamage = 0.0f;
    FTimerHandle DamageTextTimerHandle;

    void ResetDamageText();
    
protected:
    virtual void BeginPlay() override;
    
    void Die(); 
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
    UDataTable* EnemyDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
    FName EnemyDataRowName;
    
    float CurrentHP;
    float MaxHP;
    float BaseMoveSpeed;


    UPROPERTY(Transient)
    TObjectPtr<AGridManager> CachedGridManager;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float ArrivalDistance = 100.f;
    
    bool bIsDead = false;

    TArray<FVector> Waypoints;
    int32 CurrentIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UWidgetComponent> HPBarWidget;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> DeathMontage;

    int32 MyGoldReward = 0;
    int32 MyLifeDamage = 1;
    
private:
    void RemoveSlow();

    FTimerHandle SlowTimerHandle;
};