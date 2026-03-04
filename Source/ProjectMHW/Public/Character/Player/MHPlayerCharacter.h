#pragma once

#include "CoreMinimal.h"
#include "Character/MHCharacterBase.h"
#include "MHPlayerCharacter.generated.h"

UCLASS()
class PROJECTMHW_API AMHPlayerCharacter : public AMHCharacterBase
{
    GENERATED_BODY()

public:
    AMHPlayerCharacter();

    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void Interact();

    UFUNCTION(BlueprintCallable, Category = "Player")
    virtual void UsePrimaryAction();

protected:
    virtual void BeginPlay() override;
};
