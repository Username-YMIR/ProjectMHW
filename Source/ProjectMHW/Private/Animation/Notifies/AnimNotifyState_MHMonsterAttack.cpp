// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotifyState_MHMonsterAttack.h"

#include "MHGameplayTags.h"
#include "Character/Monster/MHMonsterCharacterBase.h"

void UAnimNotifyState_MHMonsterAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                   float TotalDuration)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration);

	if (!MeshComp)
	{
		return;
	}

	if (AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(MeshComp->GetOwner()))
	{
		Monster->BeginMonsterAttackWindow();
	}
}

void UAnimNotifyState_MHMonsterAttack::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	float FrameDeltaTime)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime);

	
	
	
	if (!MeshComp)
	{
		return;
	}

	AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(MeshComp->GetOwner());
	if (!Monster)
	{
		return;
	}

	const FGameplayTag FinalAttackTag =
		AttackTag.IsValid() ? AttackTag : MHGameplayTags::Ability_Monster_Attack_Basic;

	if (Monster->CanMonsterAttackHitNow())
	{
		const bool bHit = Monster->ConsumeMonsterAttackHitOnce(FinalAttackTag);
		
		Monster->ConsumeMonsterAttackHitOnce(FinalAttackTag);
	}
	
	
}

void UAnimNotifyState_MHMonsterAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::NotifyEnd(MeshComp, Animation);

	if (!MeshComp)
	{
		return;
	}

	if (AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(MeshComp->GetOwner()))
	{
		Monster->EndMonsterAttackWindow();
	}
}

FString UAnimNotifyState_MHMonsterAttack::GetNotifyName_Implementation() const
{
	
	return TEXT("MHMonsterAttack");
	
}
