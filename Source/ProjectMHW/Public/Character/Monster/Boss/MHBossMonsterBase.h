#pragma once

#include "CoreMinimal.h"
#include "Character/Monster/MHMonsterCharacterBase.h"
#include "MHBossMonsterBase.generated.h"

UCLASS()
class PROJECTMHW_API AMHBossMonsterBase : public AMHMonsterCharacterBase
{
    GENERATED_BODY()

public:
    AMHBossMonsterBase();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    virtual void StartEncounter();

    UFUNCTION(BlueprintCallable, Category = "Boss")
    virtual void RetreatToNest();

    UFUNCTION(BlueprintPure, Category = "Boss")
    bool HasEncounterStarted() const { return bEncounterStarted; }

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Boss")
    bool bEncounterStarted = false;
};
