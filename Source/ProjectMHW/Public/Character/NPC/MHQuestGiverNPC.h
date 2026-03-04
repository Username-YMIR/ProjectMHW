#pragma once

#include "CoreMinimal.h"
#include "MHNPCCharacterBase.h"
#include "MHQuestGiverNPC.generated.h"

class AMHPlayerCharacter;

UCLASS()
class PROJECTMHW_API AMHQuestGiverNPC : public AMHNPCCharacterBase
{
    GENERATED_BODY()

public:
    AMHQuestGiverNPC();

    virtual void StartInteraction(AMHPlayerCharacter* InteractingPlayer) override;

protected:
    UFUNCTION(BlueprintCallable, Category = "Quest")
    virtual void OpenQuestList();
};
