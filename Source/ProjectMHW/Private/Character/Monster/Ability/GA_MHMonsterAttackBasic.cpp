// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/Ability/GA_MHMonsterAttackBasic.h"

#include "MHGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

DEFINE_LOG_CATEGORY(GAMonsterAttack)

UGA_MHMonsterAttackBasic::UGA_MHMonsterAttackBasic()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalOnly;
	
	AbilityTags.AddTag(MHGameplayTags::Ability_Monster_Attack_Basic);

	// 어빌리티 활성 중 자동 부여
	ActivationOwnedTags.AddTag(MHGameplayTags::State_Monster_Attacking);

	
	// 포효중 블락
	ActivationBlockedTags.AddTag(MHGameplayTags::State_Monster_Roaring);
	// 공격중 블락 
	ActivationBlockedTags.AddTag(MHGameplayTags::State_Monster_Attacking);
	
}

void UGA_MHMonsterAttackBasic::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                               const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	
	
	
	
	if (!ActorInfo || !ActorInfo->AvatarActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	if (!AttackMontage)
	{
		UE_LOG(GAMonsterAttack, Warning, TEXT("MHGA_MonsterAttackBasic | AttackMontage is null"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	// todo 주석 모두 풀 기 
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(GAMonsterAttack, Warning, TEXT("MHGA_MonsterAttackBasic | CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	ClearTask();

	MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
		this,
		NAME_None,
		AttackMontage,
		1.0f,
		NAME_None,
		true
	);

	if (!MontageTask)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	MontageTask->OnCompleted.AddDynamic(this, &UGA_MHMonsterAttackBasic::OnMontageCompleted);
	MontageTask->OnInterrupted.AddDynamic(this, &UGA_MHMonsterAttackBasic::OnMontageInterrupted);
	MontageTask->OnCancelled.AddDynamic(this, &UGA_MHMonsterAttackBasic::OnMontageInterrupted);

	MontageTask->ReadyForActivation();
	
}

void UGA_MHMonsterAttackBasic::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	ClearTask();
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
	
	
	
}

void UGA_MHMonsterAttackBasic::OnMontageCompleted()
{
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

}

void UGA_MHMonsterAttackBasic::OnMontageInterrupted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);

	
}

void UGA_MHMonsterAttackBasic::ClearTask()
{
	if (MontageTask)
	{
		MontageTask->EndTask();
		MontageTask = nullptr;
	}
	
}
