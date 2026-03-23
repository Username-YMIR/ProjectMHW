#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "TimerManager.h"
#include "MHGA_MHSharpen.generated.h"

class UAbilityTask_PlayMontageAndWait;
class UAbilityTask_WaitGameplayEvent;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class PROJECTMHW_API UGA_MHSharpen : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_MHSharpen();

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

	UFUNCTION()
	void OnSharpenStart(FGameplayEventData Payload);

	UFUNCTION()
	void HandleMontageCompleted();

	UFUNCTION()
	void HandleMontageInterrupted();

	void TickSharpen();

protected:
	UPROPERTY(EditDefaultsOnly, Category="Sharpen")
	TSubclassOf<UGameplayEffect> SharpenTickEffectClass;

	UPROPERTY(EditDefaultsOnly, Category="Sharpen")
	TObjectPtr<UAnimMontage> SharpenMontage;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_WaitGameplayEvent> SharpenEventTask;

	FTimerHandle SharpenTickTimerHandle;
	bool bSharpenStarted = false;
};
