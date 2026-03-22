#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Engine/DataTable.h" // 필수!
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

    // =====================================
    // 🟢 [Public] 외부에서 마음껏 쓰는 함수들
    // =====================================
    virtual void Tick(float DeltaTime) override; // Tick은 부모를 따라 Public으로!
    
    virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

    void InitializeStats(); // 스포너가 스폰 직후에 호출해야 하므로 Public!
    void SetPath(const TArray<FVector>& NewPath); // 스포너가 수첩을 쥐여줘야 하므로 Public!
    void RecalculatePath();
    
    UFUNCTION(BlueprintCallable, Category = "Status")
    bool IsDead() const { return bIsDead; }

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnHPChangedDelegate OnHPChanged;

    void InitializeEnemy(FName InRowName);

    
protected:
    // =====================================
    // 🟡 [Protected] 나와 자식들만 쓰는 핵심 기능/변수들
    // =====================================
    virtual void BeginPlay() override;
    
    void Die(); // 체력이 0이 되면 스스로 호출하므로 Protected로 숨김!

    // 블루프린트 에디터에선 보이지만, C++ 외부에선 못 건드리는 중요 데이터!
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
    UDataTable* EnemyDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = "true"))
    FName EnemyDataRowName;

    // 내부 스탯 및 상태 변수들
    float CurrentHP;
    float MaxHP;
    bool bIsDead = false;

    // 경로 추적용 내부 수첩 (외부에선 알 필요 없음)
    TArray<FVector> Waypoints;
    int32 CurrentIndex;

    // 컴포넌트/애니메이션 (에디터 세팅용)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UWidgetComponent> HPBarWidget;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation", meta = (AllowPrivateAccess = "true"))
    TObjectPtr<UAnimMontage> DeathMontage;
};