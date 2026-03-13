// 제작자 : 허혁
// 제작일 : 2026-03-08
// 수정자 : 허혁
// 수정일 : 2026-03-13

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
	void SetAttacking(bool bNewAttacking);
	
protected:
	UPROPERTY(EditDefaultsOnly , Category="AI")
	TObjectPtr<UBehaviorTree> BehaviorTreeAsset;
	
};
