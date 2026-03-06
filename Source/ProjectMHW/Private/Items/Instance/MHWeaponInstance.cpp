// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHWeaponInstance.h"


// Sets default values
AMHWeaponInstance::AMHWeaponInstance()
{
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = WeaponMesh;
}

void AMHWeaponInstance::AssignGrantedAbilitySpecHandles(const TArray<FGameplayAbilitySpecHandle>& SpecHandles)
{
	GrantedAbilitySpecHandles = SpecHandles;
}

TArray<FGameplayAbilitySpecHandle> AMHWeaponInstance::GetGrantedAbilitySpecHandles() const
{
	return GrantedAbilitySpecHandles;
}


