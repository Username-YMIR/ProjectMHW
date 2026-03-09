// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-06
#pragma once

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "MHMonsterCharacterBase.generated.h"

class UMHMonsterAttributeSet;
class UAnimMontage;

DECLARE_LOG_CATEGORY_EXTERN(MonsterCharacter, Log, All);

UCLASS()
class PROJECTMHW_API AMHMonsterCharacterBase : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHMonsterCharacterBase();

protected:
    virtual void BeginPlay() override;

protected:
    // =========================
    // Monster State
    // =========================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Monster|State")
    bool bInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Monster|State")
    bool bHasRoared = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Monster|State")
    TObjectPtr<AActor> CombatTarget = nullptr;

    FTimerHandle SightDetectTimer;

protected:
    // =========================
    // Sight / Roar Config
    // =========================
    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightDetectInterval = 0.2f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightDetectRange = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    FName HeadLookSocketName = TEXT("HeadLookSocket");

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightHorizontalHalfAngleDeg = 60.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightVerticalHalfAngleDeg = 100.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    bool bSightRequireLineOfSight = true;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightTargetHeightOffset = 60.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float AutoPassCloseRange = 250.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Roar")
    TObjectPtr<UAnimMontage> RoarMontage = nullptr;

protected:
    // =========================
    // GAS / Attribute
    // =========================
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Monster|Attribute")
    TObjectPtr<UMHMonsterAttributeSet> MonsterAttributes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Monster|GAS")
    bool bMonsterGASInitialized = false;

protected:
    // =========================
    // Combat Entry
    // =========================
    void SetCombatTarget(AActor* NewTarget);

    void EnterCombat();          // 기존 함수 유지
    void ExitCombat();           // 기존 함수 유지
    void EnterCombatPhase();     // 공통 전투 진입
    bool IsUnaware() const;

protected:
    // =========================
    // Sight Detection
    // =========================
    void StartSightDetection();
    void StopSightDetection();
    void CheckSightDetection();

    bool CanSeeTargetFromHead(AActor* Target) const;

protected:
    // =========================
    // Event Handlers
    // =========================
    void HandleSightDetected(AActor* Target);
    void HandleDamagedFromUnaware(AActor* InstigatorActor);

protected:
    // =========================
    // Roar
    // =========================
    void StartRoar();
    UFUNCTION(BlueprintCallable, Category="Monster|Roar")
    void OnRoarFinished();

protected:
    // =========================
    // GAS Init
    // =========================
    void InitMonsterGAS();
    void ApplyStartupLooseTags();
    void GrantStartupAbilities();
    void ApplyStartupEffects();

public:
    // =========================
    // External Hook
    // =========================
    // 데미지 처리 지점에서 불러줄 함수
    void NotifyDamagedFrom(AActor* InstigatorActor);
};