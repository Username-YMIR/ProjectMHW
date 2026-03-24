// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Notifies/AnimNotify_Drink.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "MHGameplayTags.h"

void UAnimNotify_Drink::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* Owner = MeshComp ? MeshComp->GetOwner() : nullptr)
	{
		FGameplayEventData Payload;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, MHGameplayTags::Event_Item_Drink, Payload);
	}
}

FString UAnimNotify_Drink::GetNotifyName_Implementation() const
{
	return TEXT("Drink");
}