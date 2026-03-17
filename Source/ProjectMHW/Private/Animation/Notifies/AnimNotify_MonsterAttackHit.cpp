// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_MonsterAttackHit.h"

#include "MHGameplayTags.h"
#include "Character/Monster/MHMonsterCharacterBase.h"

class AMHMonsterCharacterBase;

void UAnimNotify_MonsterAttackHit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	if (!MeshComp)
	{
		return;
	}

	AActor* OwnerActor = MeshComp->GetOwner();
	if (!OwnerActor)
	{
		return;
	}

	AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(OwnerActor);
	if (!Monster)
	{
		return;
	}

	const FGameplayTag FinalAttackTag =
		AttackTag.IsValid() ? AttackTag : MHGameplayTags::Ability_Monster_Attack_Basic;

	Monster->ConsumeMonsterAttackHitOnce(FinalAttackTag);
	
	
}

FString UAnimNotify_MonsterAttackHit::GetNotifyName_Implementation() const
{
	return TEXT("MH Monster Attack Hit");
	
	
}
