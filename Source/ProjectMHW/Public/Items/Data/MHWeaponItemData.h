// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Data/MHEquipItemData.h"
#include "MHWeaponItemData.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHWeaponItemData : public UMHEquipItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0"))
	float AttackPower = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0", UIMin="0"))
	float Sharpness = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float Affinity = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(Categories="Element"))
	FGameplayTag AttackElementTag;
};
