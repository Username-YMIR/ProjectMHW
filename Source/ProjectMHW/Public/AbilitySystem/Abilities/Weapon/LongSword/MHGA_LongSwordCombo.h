#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "Weapons/Common/MHWeaponComboTypes.h"
#include "MHGA_LongSwordCombo.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHGALSCombo, Log, All);

class AMHPlayerCharacter;
class AMHLongSwordInstance;
class UMHLongSwordComboComponent;
class UAbilityTask_PlayMontageAndWait;
struct FMHLongSwordComboNode;

// 태도 콤보 어빌리티
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
    // 다음 모션 재생
    bool PlayNextMove(AMHPlayerCharacter* Player, AMHLongSwordInstance* Weapon, UMHLongSwordComboComponent* ComboComp, EMHComboInputType InputType);

    // 몽타주 종료 처리
    UFUNCTION()
    void OnMontageCompleted();

    UFUNCTION()
    void OnMontageInterrupted();

    // 현재 태스크 정리
    void ClearTask();

private:
    UPROPERTY(Transient)
    TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask; // 몽타주 태스크

    UPROPERTY(Transient)
    TObjectPtr<UMHLongSwordComboComponent> CachedComboComponent; // 콤보 컴포넌트

    UPROPERTY(Transient)
    TObjectPtr<AMHPlayerCharacter> CachedPlayer; // 플레이어

    UPROPERTY(Transient)
    TObjectPtr<AMHLongSwordInstance> CachedWeapon; // 무기
};
