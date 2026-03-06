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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Weapon", meta=(ClampMin="0.0"))
	float AttackPower = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Weapon", meta=(ClampMin="0.0"))
	EMHSharpnessColor MaxSharpnessColor = EMHSharpnessColor::Red;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Weapon", meta=(ClampMin="0", UIMin="0"))
	FMHSharpnessData SharpnessLength; 

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Weapon", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Affinity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Weapon", meta=(Categories="Element"))
	FGameplayTag AttackElementTag;
};
