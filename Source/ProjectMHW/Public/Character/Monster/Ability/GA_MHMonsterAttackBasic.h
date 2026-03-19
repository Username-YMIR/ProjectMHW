// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MHMonsterAttackBasic.generated.h"

/**
 * 
 */

class UAnimMontage;
class UAbilityTask_PlayMontageAndWait;

DECLARE_LOG_CATEGORY_EXTERN(GAMonsterAttack, Log, All);

UCLASS()
class PROJECTMHW_API UGA_MHMonsterAttackBasic : public UGameplayAbility
{
	GENERATED_BODY()
	
public:
	UGA_MHMonsterAttackBasic();
	
protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;
	
	
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly , Category="Attack")
	TObjectPtr<UAnimMontage> AttackMontage = nullptr;
	
	UPROPERTY(Transient)
	TObjectPtr<UAbilityTask_PlayMontageAndWait> MontageTask;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	void ClearTask();
	
	FGameplayTag ResolveAttackTag(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const;
	
};
