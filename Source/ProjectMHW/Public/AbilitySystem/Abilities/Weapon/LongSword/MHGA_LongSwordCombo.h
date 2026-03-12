#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayTagContainer.h"
#include "MHGA_LongSwordCombo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGALSCombo, Log, All);

class AMHPlayerCharacter;
class AMHLongSwordInstance;
class UMHLongSwordComboComponent;
class UAbilityTask_PlayMontageAndWait;
struct FMHLongSwordComboNode;

UCLASS()
class PROJECTMHW_API UMHGA_LongSwordCombo : public UGameplayAbility
{
    GENERATED_BODY()

public:
    UMHGA_LongSwordCombo();

protected:
    virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

    virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
        const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

private:
    bool PlayNextMove(
        AMHPlayerCharacter* Player,
        AMHLongSwordInstance* Weapon,
        UMHLongSwordComboComponent* ComboComp,
        const FGameplayTag& InPatternTag);

    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageInterrupted();

    void ClearTask();

private:
    UPROPERTY(Transient)
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

    UPROPERTY(Transient)
    TObjectPtr<UMHLongSwordComboComponent> CachedComboComponent;

    UPROPERTY(Transient)
    TObjectPtr<AMHPlayerCharacter> CachedPlayer;

    UPROPERTY(Transient)
    TObjectPtr<AMHLongSwordInstance> CachedWeapon;
};
