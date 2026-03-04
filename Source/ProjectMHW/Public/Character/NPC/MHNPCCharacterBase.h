#pragma once

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "MHNPCCharacterBase.generated.h"

class AMHPlayerCharacter;

UCLASS()
class PROJECTMHW_API AMHNPCCharacterBase : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHNPCCharacterBase();

    UFUNCTION(BlueprintCallable, Category = "NPC")
    virtual void StartInteraction(AMHPlayerCharacter* InteractingPlayer);
};
