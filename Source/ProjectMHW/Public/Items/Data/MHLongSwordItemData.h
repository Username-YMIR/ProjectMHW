// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHMeleeWeaponItemData.h"
#include "MHLongSwordItemData.generated.h"

UCLASS()
class PROJECTMHW_API UMHLongSwordItemData : public UMHMeleeWeaponItemData
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	UMHLongSwordItemData();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	TSoftObjectPtr<USkeletalMesh> SayaMeshData;
};
