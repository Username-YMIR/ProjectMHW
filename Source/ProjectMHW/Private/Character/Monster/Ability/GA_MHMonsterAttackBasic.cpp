// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/Ability/GA_MHMonsterAttackBasic.h"

#include "MHGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Character/Monster/MHMonsterCharacterBase.h"

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
	
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		UE_LOG(GAMonsterAttack, Warning, TEXT("MHGA_MonsterAttackBasic | CommitAbility failed"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	if (AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(ActorInfo->AvatarActor.Get()))
	{
		UE_LOG(GAMonsterAttack, Warning, TEXT("sssssssssssssssssss"));
		Monster->SetMonsterAttacking(true);
		Monster->FaceCombatTargetInstant();
		// 타격 지점 윈도우 열기 
		Monster->BeginMonsterAttackWindow();
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
	
	if (ActorInfo && ActorInfo->AvatarActor.IsValid())
	{
		if (AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(ActorInfo->AvatarActor.Get()))
		{
			// 몬스터 타격지점 윈도우 닫기 
			Monster->EndMonsterAttackWindow();
			// todo  
			Monster->SetMonsterAttacking(false);
		}
	}
	
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
