#pragma once

#include "CoreMinimal.h"
#include "Shakes/LegacyCameraShake.h"
#include "mh_hit_enemy_camera_shake.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogMHHitEnemyCameraShake, Log, All);

UCLASS(Blueprintable)
class PROJECTMHW_API UMHHitEnemyCameraShake : public ULegacyCameraShake
{
    GENERATED_BODY()

public:
    UMHHitEnemyCameraShake();
};
