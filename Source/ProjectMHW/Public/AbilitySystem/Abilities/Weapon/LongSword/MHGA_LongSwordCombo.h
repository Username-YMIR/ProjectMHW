#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "Weapons/LongSword/MHLongSwordComboGraph.h"
#include "TimerManager.h"
#include "MHGA_LongSwordCombo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGALSCombo, Log, All);

class AMHPlayerCharacter;
class AMHLongSwordInstance;
class UMHLongSwordComboComponent;
class UAbilityTask_PlayMontageAndWait;
class UAnimMontage;

UCLASS()
class PROJECTMHW_API UMHGA_LongSwordCombo : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMHGA_LongSwordCombo();
    bool TryEvaluateEarlyTransitionNow();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
#pragma region DamageSystem_GJ
protected:
    /** 외부 시스템에서 현재 콤보 어빌리티를 조기 종료할 때 호출_이건주 */
    UFUNCTION(BlueprintCallable, Category="Ability|LongSword")
    void RequestExternalEndAbility(bool bInWasCancelled = true);

private:
    /** 현재 콤보 노드 기준 DamageSpec을 생성해서 무기 인스턴스에 전달 _이건주*/
    bool PushDamageSpecToWeapon(const FMHLongSwordComboNode& InNode);

    /** 무기 인스턴스에 전달된 현재 공격 데이터를 초기화 */
    void ClearCurrentAttackDataFromWeapon();

    /** Source ASC의 Attribute 값을 읽어 현재 노드 기준 DamageSpec 생성 _이건주*/
    bool BuildDamageSpecForNode(
        const FMHLongSwordComboNode& InNode,
        FGameplayEffectSpecHandle& OutSpecHandle,
        FGameplayTag& OutAttackTag) const;

    /** 공격 원본 스탯을 ASC에서 읽는다 _이건주*/
    float GetSourceAttributeMagnitude(const FGameplayAttribute& InAttribute) const;
    
    
private:
    /** GA가 생성할 기본 대미지 GE 클래스 */
    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    /** Source ASC에서 읽을 공격 Attribute */
    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage")
    FGameplayAttribute SourcePhysicalAttackAttribute;

    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage")
    FGameplayAttribute SourceElementAttackAttribute;

    /** SetByCaller 태그 */
    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage")
    FGameplayTag PhysicalDamageDataTag;

    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage")
    FGameplayTag ElementDamageDataTag;

    /** 기술별 계수 전 공통 배율 */
    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage", meta=(ClampMin="0.0"))
    float GlobalPhysicalScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category="Ability|Damage", meta=(ClampMin="0.0"))
    float GlobalElementScale = 1.0f;
    
#pragma endregion 
    
private:
    /** 입력 패턴을 사용해 다음 기술을 선택하고 재생한다. */
    bool PlayNextMove(
        AMHPlayerCharacter* Player,
        AMHLongSwordInstance* Weapon,
        UMHLongSwordComboComponent* ComboComp,
        const FGameplayTag& InPatternTag);

    /** 선택된 노드의 몽타주를 실제로 재생하고, 전이 감시 상태를 갱신한다. */
    bool PlayResolvedNode(const FMHLongSwordComboNode& InNode, const FGameplayTag& InPreviousMoveTag);

    /** 현재 몽타주의 진행 상태를 감시하며 조기 전환 가능 시점을 판정한다. */
    void PollQueuedComboTransition();

    /** 현재 노드 설정을 기준으로 다음 기술로 조기 전환을 시도한다. */
    bool TryCommitQueuedComboTransition();

    /** 현재 상태가 조기 전환 시도를 허용하는지 확인한다. */
    bool CanEvaluateEarlyTransition() const;

    /** 현재 MoveTag가 드로우 엔트리 기술인지 확인한다. */
    bool IsDrawEntryMoveTag(const FGameplayTag& InMoveTag) const;

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageInterrupted();

    /** 현재 AbilityTask delegate를 해제하고 정리한다. */
    void ClearTask();

    /** 조기 전환 폴링 타이머를 정리한다. */
    void ClearTransitionPollingTimer();
    
    // 무기의 히트박스 콜리전을 강제로 비활성화_이건주
    // 몽타주 완료, 끊김, 어빌리티 종료 시 안전 장치
    // 정상적으로는 애님스테이트에서 관리
    void ResetMeleeWeaponAttack();

private:
    UPROPERTY(Transient)
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY(Transient)
    TObjectPtr<UMHLongSwordComboComponent> CachedComboComponent;

    UPROPERTY(Transient)
    TObjectPtr<AMHPlayerCharacter> CachedPlayer;

    UPROPERTY(Transient)
    TObjectPtr<AMHLongSwordInstance> CachedWeapon;

    UPROPERTY(Transient)
    TObjectPtr<UAnimMontage> ActiveMontage;

    UPROPERTY(Transient)
    FMHLongSwordComboNode ActiveNode;

    UPROPERTY(Transient)
    bool bHasActiveNode = false;

    UPROPERTY(Transient)
    bool bIgnoreMontageCallbacks = false;

    FTimerHandle TransitionPollingTimerHandle;
};