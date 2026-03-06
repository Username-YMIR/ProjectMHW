// 제작자 : 손승우
// 제작일 : 2026-03-04
// 수정자 : 허혁
// 수정일 : 2026-03-05

#pragma once

DECLARE_LOG_CATEGORY_EXTERN(MonsterCharacter, Log, All)

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "Character/Monster/Attribute/MHMonsterAttributeSet.h"
#include "MHMonsterCharacterBase.generated.h"

class UAbilitySystemComponent;

UCLASS()
class PROJECTMHW_API AMHMonsterCharacterBase : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHMonsterCharacterBase();

    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void SetCombatTarget(AActor* NewTarget);

    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void EnterCombat();

    UFUNCTION(BlueprintCallable, Category = "Monster")
    virtual void ExitCombat();

    UFUNCTION(BlueprintPure, Category = "Monster")
    bool IsInCombat() const { return bInCombat; }

    UFUNCTION(BlueprintPure, Category = "Monster")
    AActor* GetCombatTarget() const { return CombatTarget; }

    virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override
    {return AbilitySystemComponent; }
    
    
protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
    bool bInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
    TObjectPtr<AActor> CombatTarget = nullptr;
    
    UPROPERTY(VisibleAnywhere , BlueprintReadOnly , Category="GAS" , meta=(AllowPrivateAccess=true))
    TObjectPtr<UMHMonsterAttributeSet> MonsterAttributes;
    
    
    
    void InitMonsterGAS();
    void ApplyStartupLooseTags();
    void GrantStartupAbilities();
    void ApplyStartupEffects();
    

};
