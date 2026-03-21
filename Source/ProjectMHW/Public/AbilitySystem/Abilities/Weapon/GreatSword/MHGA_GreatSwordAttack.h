#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "MHGA_GreatSwordAttack.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGAGreatSwordAttack, Log, All);

class AMHGreatSwordInstance;
class AMHPlayerCharacter;
class UAbilityTask_PlayMontageAndWait;
class UGameplayEffect;

UCLASS()
class PROJECTMHW_API UMHGA_GreatSwordAttack : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMHGA_GreatSwordAttack();

protected:
    virtual void ActivateAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(
        const FGameplayAbilitySpecHandle Handle,
        const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo,
        bool bReplicateEndAbility,
        bool bWasCancelled) override;

    // 현재 대검 액션 컴포넌트가 선택한 기술 태그 기준으로 DamageSpec을 만든다.
    bool BuildDamageSpecForMove(const FGameplayTag& InMoveTag, FGameplayEffectSpecHandle& OutSpecHandle) const;

    // 현재 기술 태그와 DamageSpec을 무기 인스턴스에 전달한다.
    bool PushCurrentAttackDataToWeapon(const FGameplayTag& InMoveTag);

    // 무기 인스턴스에 전달한 현재 공격 데이터를 초기화한다.
    void ClearCurrentAttackDataFromWeapon();

    // 액션 종료 시 대검 런타임 상태를 정리한다.
    void FinalizeGreatSwordActionState();

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageInterrupted();

    void ClearTask();

protected:
    UPROPERTY(EditDefaultsOnly, Category = "Ability|Damage")
    TSubclassOf<UGameplayEffect> DamageEffectClass;

    UPROPERTY(EditDefaultsOnly, Category = "Ability|Damage")
    FGameplayAttribute SourcePhysicalAttackAttribute;

    UPROPERTY(EditDefaultsOnly, Category = "Ability|Damage")
    FGameplayTag PhysicalDamageDataTag;

    UPROPERTY(EditDefaultsOnly, Category = "Ability|Damage", meta = (ClampMin = "0.0"))
    float GlobalPhysicalScale = 1.0f;

    UPROPERTY(Transient)
    TObjectPtr<AMHPlayerCharacter> CachedPlayer;

    UPROPERTY(Transient)
    TObjectPtr<AMHGreatSwordInstance> CachedWeapon;

    UPROPERTY(Transient)
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY(Transient)
    FGameplayTag ActiveMoveTag;
};
