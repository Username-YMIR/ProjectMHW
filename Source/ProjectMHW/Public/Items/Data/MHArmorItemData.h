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
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DefensePower = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float FireResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float WaterResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ThunderResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float IceResist = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DragonResist = 0.f;
};
