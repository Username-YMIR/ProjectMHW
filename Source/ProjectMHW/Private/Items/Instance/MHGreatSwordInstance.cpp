// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Instance/MHGreatSwordInstance.h"


// Sets default values
AMHGreatSwordInstance::AMHGreatSwordInstance()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	WeaponType = EMHWeaponType::GreatSword; // 손승우 추가

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


