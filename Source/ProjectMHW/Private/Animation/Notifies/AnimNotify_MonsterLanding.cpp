// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_MonsterLanding.h"

#include "Character/Monster/MHMonsterCharacterBase.h"

void UAnimNotify_MonsterLanding::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!MeshComp)
	{
		return;
	}

	if (AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(MeshComp->GetOwner()))
	{
		Monster->ExecuteChargeP2ImpactSnap();
	}
	
	
}
