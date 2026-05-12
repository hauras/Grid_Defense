#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" 
#include "GameplayTagContainer.h"
#include "Data/ElementalData.h"
#include "EnemyBase.generated.h"

class ANexus;
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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    int32 GoldReward = 10;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    int32 LifeDamage = 1;

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
    
    // [수정] TObjectPtr로 변경
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UDataTable> EnemyDataTable;

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

    // 상성 DataTable
    UPROPERTY(EditDefaultsOnly, Category = "Combat")
    TObjectPtr<UDataTable> ElementalMatchupTable;
    
private:
    void RemoveSlow();

    FTimerHandle SlowTimerHandle;
    FTimerHandle NexusAttackTimerHandle;

    UFUNCTION()
    void AttackNexus();

    bool bIsAttackingNexus = false;
};