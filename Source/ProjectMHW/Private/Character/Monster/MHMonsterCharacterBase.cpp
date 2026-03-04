#include "Character/Monster/MHMonsterCharacterBase.h"

AMHMonsterCharacterBase::AMHMonsterCharacterBase()
{
}

void AMHMonsterCharacterBase::BeginPlay()
{
    Super::BeginPlay();
}

void AMHMonsterCharacterBase::SetCombatTarget(AActor* NewTarget)
{
    CombatTarget = NewTarget;
}

void AMHMonsterCharacterBase::EnterCombat()
{
    bInCombat = true;
}

void AMHMonsterCharacterBase::ExitCombat()
{
    bInCombat = false;
    CombatTarget = nullptr;
}
