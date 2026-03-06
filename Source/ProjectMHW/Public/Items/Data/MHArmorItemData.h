// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Data/MHEquipItemData.h"
#include "MHArmorItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHArmorItemData : public UMHEquipItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Armor")
	float DefensePower = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Armor")
	float FireResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Armor")
	float WaterResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Armor")
	float ThunderResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Armor")
	float IceResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Item|Armor")
	float DragonResist = 0.f;
};
