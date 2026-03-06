// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHEquipItemInstance.h"
#include "GameplayAbilitySpecHandle.h"
#include "MHWeaponInstance.generated.h"

UCLASS()
class PROJECTMHW_API AMHWeaponInstance : public AMHEquipItemInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHWeaponInstance();


protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Weapon")
	USkeletalMeshComponent* WeaponMesh;

	//TODO: 프로젝트용 구조체로 변경하기 _ 이건주 
	// UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="WeaponData")
	// FHeroWeaponData HeroWeaponData;
};
