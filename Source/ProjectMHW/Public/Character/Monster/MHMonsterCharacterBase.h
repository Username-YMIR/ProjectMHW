#pragma once

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "MHMonsterCharacterBase.generated.h"

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

protected:
    virtual void BeginPlay() override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
    bool bInCombat = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Monster")
    TObjectPtr<AActor> CombatTarget = nullptr;
};
