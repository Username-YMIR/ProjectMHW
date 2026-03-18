// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AnimNotify_MonsterAttackHit.generated.h"

/**
 * 
 */
UCLASS(meta=(DisplayName="MH Monster Attack Hit"))
class PROJECTMHW_API UAnimNotify_MonsterAttackHit : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere , BlueprintReadWrite , Category="Monster|Attack")
	FGameplayTag AttackTag; 
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
	virtual FString GetNotifyName_Implementation() const override;
	
};
