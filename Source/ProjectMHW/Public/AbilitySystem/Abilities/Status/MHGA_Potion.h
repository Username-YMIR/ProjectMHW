#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "MHGA_Potion.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class PROJECTMHW_API UMHGA_Potion : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UMHGA_Potion();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnDrink(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	void TickHeal();

	UPROPERTY(EditDefaultsOnly, Category="Potion")
	TSubclassOf<UGameplayEffect> PotionHealableEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Potion")
	TSubclassOf<UGameplayEffect> PotionHealTickEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Potion")
	TObjectPtr<UAnimMontage> PotionMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> DrinkEventTask;

	FTimerHandle HealTickTimerHandle;
	bool bDrinkTriggered = false;
};
