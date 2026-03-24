// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_SharpenStart.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "MHGameplayTags.h"
#include "Abilities/GameplayAbilityTypes.h"

void UAnimNotify_SharpenStart::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		FGameplayEventData Payload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, MHGameplayTags::Event_Item_SharpenStart, Payload);
	}
}

FString UAnimNotify_SharpenStart::GetNotifyName_Implementation() const
{
	return TEXT("SharpenStart");
}