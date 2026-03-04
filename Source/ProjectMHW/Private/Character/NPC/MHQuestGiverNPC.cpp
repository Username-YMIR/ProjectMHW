#include "Character/NPC/MHQuestGiverNPC.h"

#include "Character/Player//MHPlayerCharacter.h"

AMHQuestGiverNPC::AMHQuestGiverNPC()
{
}

void AMHQuestGiverNPC::StartInteraction(AMHPlayerCharacter* InteractingPlayer)
{
    Super::StartInteraction(InteractingPlayer);
    OpenQuestList();
}

void AMHQuestGiverNPC::OpenQuestList()
{
}
