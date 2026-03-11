// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Monster/AI/MHMonsterAIController.h"
#include "Character/Monster/AI/MHMonsterBlackboardKeys.h"
#include "BehaviorTree/BlackboardComponent.h"

AMHMonsterAIController::AMHMonsterAIController()
{
}

void AMHMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (BehaviorTreeAsset)
	{
		const bool bRan = RunBehaviorTree(BehaviorTreeAsset);
		
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("AIController | BehaviorTreeAsset is null"));
	}
}

void AMHMonsterAIController::SetCombatTarget(AActor* NewTarget)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		
		BB->SetValueAsObject(MHMonsterBBKeys::TargetActor, NewTarget);
	}
}

void AMHMonsterAIController::SetInCombat(bool bNewInCombat)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		
		BB->SetValueAsBool(MHMonsterBBKeys::bInCombat, bNewInCombat);
	}
}

void AMHMonsterAIController::SetIsRoaring(bool bNewIsRoaring)
{
	if (UBlackboardComponent* BB = GetBlackboardComponent())
	{
		
		BB->SetValueAsBool(MHMonsterBBKeys::bIsRoaring, bNewIsRoaring);
	}
}
