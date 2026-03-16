// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-09
#pragma once

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "GameplayTagContainer.h"
#include "MHMonsterCharacterBase.generated.h"

class UMHMonsterAttributeSet;
class UAnimMontage;
class AMHMonsterAIController;

DECLARE_LOG_CATEGORY_EXTERN(MHMonsterCharacterBase, Log, All);

UCLASS()
class PROJECTMHW_API AMHMonsterCharacterBase : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHMonsterCharacterBase();

protected:
    virtual void BeginPlay() override;

public:
    UFUNCTION(BlueprintPure , Category="Monster|Combat")
    AActor* GetCombatTarget() const {return CombatTarget;}
    
    UFUNCTION(BlueprintPure , Category="Monster|Combat")
    bool IsInCombat() const { return bInCombat;}
    
    UFUNCTION(BlueprintPure , Category="Monster|Combat")
    bool HasRoared() const { return bHasRoared;}
    
    UFUNCTION(BlueprintCallable, Category="Monster|Combat")
    bool TryActivateMonsterAbilityByTag(FGameplayTag AbilityTag);
    
    UFUNCTION(BlueprintPure, Category="Monster|Combat")
    float GetDistanceToCombatTarget() const;
    
    UFUNCTION(BlueprintPure, Category="Monster|Combat")
    bool IsCombatTargetInRange(float Range) const;
    
    
    
#pragma region DamageSystem_GJ
public:
    virtual FMHHitAcknowledge ReceiveDamageSpec_Implementation(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FGameplayEffectSpecHandle& DamageSpecHandle,
        const FHitResult& HitResult
    ) override;

protected:
    virtual void HandleDamageAccepted(
        AActor* SourceActor,
        AActor* SourceWeapon,
        FGameplayTag AttackTag,
        const FHitResult& HitResult
    ) override;

    virtual void HandleDeath() override;

    /** 공격 메타 테이블에서 VFX / SFX를 찾아 실행 */
    virtual void PlayHitImpactFXByAttackTag(
        FGameplayTag AttackTag,
        const FHitResult& HitResult
    );

    /** 공격 메타 테이블에서 사운드를 찾아 실행 */
    virtual void PlayHitSoundByAttackTag(
        FGameplayTag AttackTag,
        const FHitResult& HitResult
    );

protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Combat|Data")
    TObjectPtr<UDataTable> AttackMetaTable = nullptr;
#pragma endregion  
    
    
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
    
    // =========================
    // Monster Controller
    // ========================= 

   
    AMHMonsterAIController* GetMonsterAIController() const;
#pragma region Ambient
    // =========================
    //  Ambient 
    // =========================
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Ambient")
    float AmbientWalkSpeed = 120.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Ambient")
    float CombatWalkSpeed = 260.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Ambient")
    float AmbientMoveRadius = 500.f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Ambient")
    float AmbientDecisionIntervalMin = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Ambient")
    float AmbientDecisionIntervalMax = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Ambient")
    float AmbientIdleChance = 0.5f;
    
protected:
    
    FTimerHandle AmbientBehaviorTimer;

    void StartAmbientBehavior();
    void StopAmbientBehavior();
    void ScheduleNextAmbientDecision();
    void DecideNextAmbientAction();
    void MoveToRandomAmbientLocation();

    UFUNCTION(BlueprintCallable, Category="Monster|Movement")
    void SetMonsterMoveSpeed(float NewSpeed);
    
    
    
#pragma endregion
    
    
    
    
protected:
#pragma region Roar
    // =========================
    // Sight / Roar Config
    // =========================
    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightDetectInterval = 0.2f;

    // 인식 거리 
    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightDetectRange = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    FName HeadLookSocketName = TEXT("HeadLookSocket");

    // 좌우 시야 반각
    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightHorizontalHalfAngleDeg = 30.f;

    //위아래 시야 반각
    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightVerticalHalfAngleDeg = 110.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    bool bSightRequireLineOfSight = true;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float SightTargetHeightOffset = 60.f;
    
    // 너무 가까울때 인식 범위
    UPROPERTY(EditDefaultsOnly, Category="Monster|Sight")
    float AutoPassCloseRange = 180.f;

    UPROPERTY(EditDefaultsOnly, Category="Monster|Roar")
    TObjectPtr<UAnimMontage> RoarMontage = nullptr;
    
    /*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Sight|Debug")
    bool bDrawSightDebug = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Monster|Sight|Debug")
    float SightDebugLineLength = 250.f;*/
    
    void DrawSightConeDebug(
    const FVector& SocketLocation,
    const FRotator& SocketRotation,
    const FVector& TargetLocation,
    bool bCanSee
) const;
    
#pragma endregion 
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

public:
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