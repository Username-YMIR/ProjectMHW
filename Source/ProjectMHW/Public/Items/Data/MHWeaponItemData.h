// Fill out your copyright notice in the Description page of Project Settings.
// 제작자 : 이건주
// 제작일 : 2026-03-05
// 수정일 : 2026-03-05 
#pragma once

#include "CoreMinimal.h"
#include "Items/Data/MHEquipItemData.h"
#include "Type/MHCombatStatStructType.h"
#include "MHWeaponItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHWeaponItemData : public UMHEquipItemData
{
	GENERATED_BODY()
public:
	UMHWeaponItemData();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Stats")
	FMHAttackStats AttackStats;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSoftObjectPtr<USkeletalMesh> WeaponMeshData;
};
