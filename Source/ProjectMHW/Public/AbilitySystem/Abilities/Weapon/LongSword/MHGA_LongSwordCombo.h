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
