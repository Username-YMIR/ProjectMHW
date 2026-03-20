// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/Ability/GA_MHMonsterAttackBasic.h"

#include "MHGameplayTags.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemComponent.h"
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

    AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(ActorInfo->AvatarActor.Get());
    if (!Monster)
    {
        UE_LOG(GAMonsterAttack, Warning, TEXT("MHGA_MonsterAttackBasic | Monster cast failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    const FGameplayTag ResolvedAttackTag = ResolveAttackTag(Handle, ActorInfo);

    FMonsterAbilityEntry AbilityEntry;
    UAnimMontage* MontageToPlay = AttackMontage; // fallback

    if (ResolvedAttackTag.IsValid() && Monster->GetMonsterAbilityEntryByTag(ResolvedAttackTag, AbilityEntry))
    {
        if (AbilityEntry.Montage)
        {
            MontageToPlay = AbilityEntry.Montage;
        }

        UE_LOG(GAMonsterAttack, Warning,
            TEXT("MHGA_MonsterAttackBasic | ResolvedTag=%s Montage=%s"),
            *ResolvedAttackTag.ToString(),
            *GetNameSafe(MontageToPlay));
    }
    else
    {
        UE_LOG(GAMonsterAttack, Warning,
            TEXT("MHGA_MonsterAttackBasic | Entry not found, fallback montage used | Tag=%s Montage=%s"),
            *ResolvedAttackTag.ToString(),
            *GetNameSafe(MontageToPlay));
    }

    if (!MontageToPlay)
    {
        UE_LOG(GAMonsterAttack, Warning, TEXT("MHGA_MonsterAttackBasic | MontageToPlay is null"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        UE_LOG(GAMonsterAttack, Warning, TEXT("MHGA_MonsterAttackBasic | CommitAbility failed"));
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    Monster->SetMonsterAttacking(true);
    Monster->FaceCombatTargetInstant();
    Monster->BeginMonsterAttackWindow();

    ClearTask();

    MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this,
        NAME_None,
        MontageToPlay,
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

FGameplayTag UGA_MHMonsterAttackBasic::ResolveAttackTag(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo) const
{
	if (!ActorInfo || !ActorInfo->AbilitySystemComponent.IsValid())
	{
		return FGameplayTag();
	}

	const FGameplayAbilitySpec* Spec =
		ActorInfo->AbilitySystemComponent->FindAbilitySpecFromHandle(Handle);

	if (!Spec || !Spec->Ability)
	{
		return FGameplayTag();
	}

	for (const FGameplayTag& DynamicTag : Spec->DynamicAbilityTags)
	{
		if (DynamicTag.IsValid())
		{
			return DynamicTag;
		}
	}

	for (const FGameplayTag& AssetTag : Spec->Ability->AbilityTags)
	{
		if (AssetTag.IsValid())
		{
			return AssetTag;
		}
	}

	return FGameplayTag();
	
}
