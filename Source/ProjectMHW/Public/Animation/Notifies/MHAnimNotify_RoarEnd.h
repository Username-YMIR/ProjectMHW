// Fill out your copyright notice in the Description page of Project Settings.

// 제작자 : 허혁
// 제작일 : 2026-03-09
// 수정자 : 허혁
// 수정일 : 2026-03-09

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MHAnimNotify_RoarEnd.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UMHAnimNotify_RoarEnd : public UAnimNotify
{
	GENERATED_BODY()
	
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
