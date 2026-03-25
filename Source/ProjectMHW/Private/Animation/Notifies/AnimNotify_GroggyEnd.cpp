// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_GroggyEnd.h"

#include "Character/Monster/MHMonsterCharacterBase.h"

void UAnimNotify_GroggyEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	if (!MeshComp)
	{
		return;
	}

	if (AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(MeshComp->GetOwner()))
	{
		Monster->FinishGroggy();
	}
}
