#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" 
#include "GameplayTagContainer.h"    
#include "EnemyBase.generated.h"

class UWidgetComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnHPChangedDelegate, float, CurrentHP, float, MaxHP);

USTRUCT(BlueprintType)
struct FEnemyData : public FTableRowBase
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MaxHP = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
    float MoveSpeed = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    USkeletalMesh* EnemyMesh;
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
    
    UFUNCTION(BlueprintCallable, Category = "Status")
    bool IsDead() const { return bIsDead; }

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHPChangedDelegate OnHPChanged;

    void InitializeEnemy(FName InRowName);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
    FGameplayTagContainer GameplayTags;

    void ApplySlow(float SlowDuration);
    
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

    
    bool bIsDead = false;

    TArray<FVector> Waypoints;
    int32 CurrentIndex;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UWidgetComponent> HPBarWidget;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> DeathMontage;
    
private:
    void RemoveSlow();

    FTimerHandle SlowTimerHandle;
};