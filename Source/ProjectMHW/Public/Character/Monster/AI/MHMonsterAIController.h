// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MHMonsterAIController.generated.h"

/**
 * 
 */

class UBehaviorTree;
class UBlackboardComponent;


UCLASS()
class PROJECTMHW_API AMHMonsterAIController : public AAIController
{
	GENERATED_BODY()
	
public:
	AMHMonsterAIController();
	
protected:
	virtual void OnPossess(APawn* InPawn) override;
	
public:
	void SetCombatTarget(AActor* NewTarget);
	void SetInCombat(bool bNewInCombat);
	void SetIsRoaring(bool bNewIsRoaring);
	
protected:
	UPROPERTY(EditDefaultsOnly , Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
	
};
