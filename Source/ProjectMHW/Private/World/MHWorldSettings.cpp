#include "World/MHWorldSettings.h"

DEFINE_LOG_CATEGORY(LogMHWorldSettings);

AMHWorldSettings::AMHWorldSettings()
{
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;
}
