// Fill out your copyright notice in the Description page of Project Settings.
// 제작자 : 허혁
// 제작일 : 2026-03-09
// 수정자 : 허혁
// 수정일 : 2026-03-09



#include "Animation/Notifies/MHAnimNotify_RoarEnd.h"
#include "Character/Monster/MHMonsterCharacterBase.h"

void UMHAnimNotify_RoarEnd::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
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

	Monster->OnRoarFinished();
	
}
