// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AnimNotifyState_MHMonsterAttack.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTMHW_API UAnimNotifyState_MHMonsterAttack : public UAnimNotifyState
{
	GENERATED_BODY()
	
	
	public:
    	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Monster Attack")
    	FGameplayTag AttackTag;
    
    	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
    	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime) override;
    	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
    
    	virtual FString GetNotifyName_Implementation() const override;
};
