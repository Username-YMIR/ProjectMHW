// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MHMeleeWeaponInstance.h"
#include "MHLongSwordInstance.generated.h"

UCLASS()
class PROJECTMHW_API AMHLongSwordInstance : public AMHMeleeWeaponInstance
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMHLongSwordInstance();
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Weapon")
	USkeletalMesh* SayaMesh;
};
