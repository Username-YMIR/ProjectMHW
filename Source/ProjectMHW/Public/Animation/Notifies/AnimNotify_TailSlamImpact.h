// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_TailSlamImpact.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UAnimNotify_TailSlamImpact : public UAnimNotify
{
	GENERATED_BODY()
	
	public:
        virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

	
};
