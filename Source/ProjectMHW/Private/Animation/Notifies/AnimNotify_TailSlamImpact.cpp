// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_TailSlamImpact.h"

#include "Character/Monster/MHMonsterCharacterBase.h"

void UAnimNotify_TailSlamImpact::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);
	
	if (!MeshComp)
	{
		return;
	}

	AMHMonsterCharacterBase* Monster = Cast<AMHMonsterCharacterBase>(MeshComp->GetOwner());
	if (!Monster)
	{
		return;
	}

	Monster->SpawnTailSlamGroundImpactFX();
}
