// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MonsterJumpStart.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="MH Monster Charge JumpStart"))
class PROJECTMHW_API UAnimNotify_MonsterJumpStart : public UAnimNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual FString GetNotifyName_Implementation() const override
	{
		return TEXT("MH Monster Charge JumpStart");
	}
};
