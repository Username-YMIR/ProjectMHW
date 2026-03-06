// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHEquipItemInstance.h"


// Sets default values
AMHEquipItemInstance::AMHEquipItemInstance()
{

}

void AMHEquipItemInstance::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles)
{
	GrantedAbilitySpecHandles = SpecHandles;
}


TArray<FGameplayAbilitySpecHandle> AMHEquipItemInstance::GetGrantedAbilitySpecHandles() const
{
	return GrantedAbilitySpecHandles;
}
