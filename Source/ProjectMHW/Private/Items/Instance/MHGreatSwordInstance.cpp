// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHGreatSwordInstance.h"

#include "Weapons/GreatSword/MHGreatSwordActionComponent.h"

// Sets default values
AMHGreatSwordInstance::AMHGreatSwordInstance()
{
	PrimaryActorTick.bCanEverTick = false;
	WeaponType = EMHWeaponType::GreatSword;

	// 대검 전용 입력 상태와 차징 상태를 관리한다.
	ActionComponent = CreateDefaultSubobject<UMHGreatSwordActionComponent>(TEXT("ActionComponent"));
}

void AMHGreatSwordInstance::ApplyItemData()
{
	Super::ApplyItemData();
	
	const UMHGreatSwordItemData* GreatSwordData = GetGreatSwordData();
	if (!GreatSwordData) return;
}

void AMHGreatSwordInstance::BeginPlay()
{
	Super::BeginPlay();
}


