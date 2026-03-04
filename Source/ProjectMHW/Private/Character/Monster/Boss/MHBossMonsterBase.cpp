#include "Character/Monster/Boss/MHBossMonsterBase.h"

AMHBossMonsterBase::AMHBossMonsterBase()
{
}

void AMHBossMonsterBase::StartEncounter()
{
    bEncounterStarted = true;
    EnterCombat();
}

void AMHBossMonsterBase::RetreatToNest()
{
    ExitCombat();
}
