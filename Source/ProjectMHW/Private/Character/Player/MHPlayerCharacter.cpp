#include "Character/Player/MHPlayerCharacter.h"

#include "Components/InputComponent.h"

AMHPlayerCharacter::AMHPlayerCharacter()
{
}

void AMHPlayerCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void AMHPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMHPlayerCharacter::Interact()
{
}

void AMHPlayerCharacter::UsePrimaryAction()
{
}
