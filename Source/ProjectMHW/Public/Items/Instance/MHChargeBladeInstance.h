// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Instance/MHMeleeWeaponInstance.h"
#include "MHChargeBladeInstance.generated.h"

UCLASS()
class PROJECTMHW_API AMHChargeBladeInstance : public AMHMeleeWeaponInstance
{
	GENERATED_BODY()

public:
	AMHChargeBladeInstance();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Weapon", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ShieldMesh; // 방패 메쉬

public:
	virtual USkeletalMeshComponent* GetSheathMeshComponent() const override { return ShieldMesh; }
};
